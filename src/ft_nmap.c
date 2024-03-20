//
// Created by loumouli on 3/19/24.
//

#include "ft_nmap.h"

uint16_t port = 22;

void ft_hexdump(const void* data, const uint64_t nbytes, const uint64_t row) {
  if (row == 0) {
    for (size_t i = 0; i < nbytes; i++) {
      printf("%02x ", ((uint8_t*)data)[i]);
    }
    printf("\n");
    return;
  }
  for (size_t i = 0; i < nbytes; i += row) {
    for (size_t j = i; j < i + row; j++) {
      if (j == nbytes) {
        break;
      }
      printf("%02x ", ((uint8_t*)data)[j]);
    }
    printf("\n");
  }
  printf("\n");
}

struct in_addr get_interface_ip(const char* ifname) {
  struct ifaddrs* ifaddr;
  struct in_addr ipAddr = {0};

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  // Walk through linked list, maintaining head pointer so we can free list later
  for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
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

uint16_t checksum(uint16_t* buffer, int size) {
  uint64_t cksum = 0;
  while (size > 1) {
    cksum += *buffer++;
    size -= sizeof(uint16_t);
  }
  if (size)
    cksum += *(uint8_t*)buffer;

  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += cksum >> 16;
  return ~cksum;
}

int main(int ac, char** av) {
  (void)ac;
  uint8_t packet[sizeof(struct tcphdr) + sizeof(TCP_PseudoHeader)] = {0};

  struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
  struct sockaddr_in dest = {0};
  const int sck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sck == -1) {
    perror("socket SOCK RAW");
    return 1;
  }
  setsockopt(sck, IPPROTO_IP, IP_HDRINCL, (uint64_t[]){1}, sizeof(uint64_t));
  tcp_hdr->source = htons(49152); // Source port is this. Likely unused port
  tcp_hdr->dest = htons(port); // Targeting port  (SSH)
  tcp_hdr->seq = 1;
  tcp_hdr->syn = htons(42);
  tcp_hdr->ack_seq = 0;
  tcp_hdr->window = 1024;
  tcp_hdr->doff = 6;
  dest.sin_family = AF_INET;
  dest.sin_port = tcp_hdr->dest;
  dest.sin_addr.s_addr = inet_addr(av[1]);
  tcp_hdr->check = checksum((uint16_t*)tcp_hdr, sizeof(tcp_hdr));

  int64_t retval = sendto(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest, sizeof(dest));
  if (retval == -1) {
    perror("sendto");
    close(sck);
    return 1;
  }
  printf("%ld bytes sent\n", retval);
  struct sockaddr_in answer = {0};
  socklen_t len = 0;
  uint8_t buff[4096] = {0};
  retval = recvfrom(sck, buff, sizeof(buff), 0, (struct sockaddr*)&answer, &len);
  if (retval == -1) {
    perror("recvfrom");
    close(sck);
    return 1;
  }

  printf("%ld bytes recv\n", retval);
  struct iphdr* ip_hdr = (void*)buff;
  printf("full hexdump:\n");
  ft_hexdump(buff, retval, 0);
  printf("hexdump ip header:\n");
  ft_hexdump(ip_hdr, sizeof(struct iphdr), 4);
  if (ip_hdr->protocol == IPPROTO_TCP) {
    printf("its a tcp packet, youhou !\n");
  }
  const struct tcphdr* tcp_headr_recv = (void*)buff + sizeof(struct iphdr);
  printf("hexdump tcp header:\n");
  ft_hexdump(tcp_headr_recv, sizeof(struct tcphdr), 4);
  printf("\nURGENT = %d, ACK = %d, PUSH = %d, RESET = %d, SYN = %d, FIN = %d\n", tcp_headr_recv->urg, tcp_headr_recv->ack, tcp_headr_recv->psh, tcp_headr_recv->rst, tcp_headr_recv->syn, tcp_headr_recv->fin);
  if (tcp_headr_recv->ack == 1 && tcp_headr_recv->syn == 1) {
    printf("ACK recv, youhou !\n");
    printf("syn seq = %u\n", ntohs(tcp_headr_recv->seq));
    printf("RST sent\nPort %d is open\n", port);
  }

  // need to send back, tcp packet with RESET bit set
  tcp_hdr->syn = 0;
  tcp_hdr->rst = 1;
  retval = sendto(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest, sizeof(dest));
  if (retval == -1) {
    perror("sendto");
    close(sck);
    return 1;
  }
  close(sck);
  return 0;
}
