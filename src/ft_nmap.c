//
// Created by loumouli on 3/19/24.
//

#include "ft_nmap.h"

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
  cksum += (cksum >> 16);
  return ~cksum;
}

uint16_t tcp_checksum(void* vdata, size_t length, struct in_addr src_addr, struct in_addr dest_addr) {
  // Create the pseudo header
  TCP_PseudoHeader psh = {0};
  psh.src_addr = src_addr.s_addr;
  psh.dest_addr = dest_addr.s_addr;
  psh.pholder = 0;
  psh.protocol = IPPROTO_TCP;
  psh.tcp_len = htons(length);

  // Calculate the size for the pseudo header + TCP header
  const size_t psize = sizeof(TCP_PseudoHeader) + length;
  char* pseudogram = malloc(psize);
  if (pseudogram == NULL)
    return 0;

  // Copy the pseudo header first
  memcpy(pseudogram, (char*)&psh, sizeof(TCP_PseudoHeader));

  // Then copy the TCP header (and data if any)
  memcpy(pseudogram + sizeof(TCP_PseudoHeader), vdata, length);

  // Calculate the checksum
  uint16_t chcksm = checksum((uint16_t*)pseudogram, psize);
  free(pseudogram);
  return chcksm;
}


int main(int ac, char** av) {
  (void)ac;
  uint8_t packet[sizeof(struct tcphdr) + sizeof(TCP_PseudoHeader)] = {0};

  struct tcphdr* tcphdr = (struct tcphdr*)&packet;
  struct sockaddr_in dest = {0};
  const int sck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sck == -1) {
    perror("socket SOCK RAW");
    return 1;
  }
  setsockopt(sck, IPPROTO_IP, IP_HDRINCL, (uint64_t[]){1}, sizeof(uint64_t));
  tcphdr->source = htons(49152); // Source port is this. Likely unused port
  tcphdr->dest = htons(22); // Targeting port 22 (SSH)
  tcphdr->seq = 1;
  tcphdr->ack_seq = 1;
  tcphdr->syn = 1;
  tcphdr->window = 1;
  dest.sin_family = AF_INET;
  dest.sin_port = tcphdr->dest;
  dest.sin_addr.s_addr = inet_addr(av[1]);
  tcphdr->check = tcp_checksum((uint16_t*)tcphdr, sizeof(tcphdr), get_interface_ip("eth0"), dest.sin_addr);

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
  close(sck);
  return 0;
}
