//
// Created by loumouli on 3/19/24.
//

#include "ft_nmap.h"

uint16_t port = 80;

struct in_addr get_interface_ip(const char* ifname) {
  struct ifaddrs* ifaddr;
  const struct in_addr ipAddr = {0};

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  // Walk through linked list, maintaining head pointer so we can free list later
  for (const struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (strcmp(ifa->ifa_name, ifname) == 0 && ifa->ifa_addr->sa_family == AF_INET) { // Check it is IP4
      // is a valid IP4 Address
      freeifaddrs(ifaddr);
      return ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
    }
  }

  freeifaddrs(ifaddr);
  return ipAddr;
}

int main(int ac, char** av) {
  (void)ac;
  uint8_t packet[sizeof(struct tcphdr)] = {0};

  struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
  struct sockaddr_in dest = {0};
  const int sck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sck == -1) {
    perror("socket SOCK RAW");
    return 1;
  }
  tcp_syn_craft_payload(tcp_hdr, port);
  dest.sin_family = AF_INET;
  dest.sin_port = tcp_hdr->dest;
  dest.sin_addr.s_addr = inet_addr(av[1]);
  tcp_hdr->check = tcp_checksum((uint16_t*)tcp_hdr, sizeof(struct tcphdr), get_interface_ip("eth0"), dest.sin_addr);
  int64_t retval = send_packet(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest);
  if (retval == -1) {
    close(sck);
    return 1;
  }
  uint8_t buff[4096] = {0};
  struct sockaddr_in sender = {0};
  retval = recv_packet(sck, buff, sizeof(buff), 0, (struct sockaddr*)&sender);
  if (retval == -1) {
    close(sck);
    return 1;
  }

  const struct iphdr* ip_hdr = (void*)buff;
  if (ip_hdr->protocol == IPPROTO_TCP) {
  }
  switch (tcp_syn_analysis(NULL, (void*)buff + sizeof(struct iphdr))) {
  case OPEN:
    printf("Port %d is open\n", port);
    break;
  case CLOSE:
    printf("Port %d is close\n", port);
    break;
  case FILTERED:
    printf("Port %d is filtered\n", port);
    break;
  default: {
  }
  }
  // need to send back, tcp packet with RESET bit set
  retval = tcp_syn_cleanup(sck, packet, sizeof(packet), 0, (void*)&dest);
  if (retval == -1) {
    close(sck);
    return 1;
  }
  close(sck);
  return 0;
}
