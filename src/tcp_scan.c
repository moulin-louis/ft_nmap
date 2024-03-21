//
// Created by loumouli on 3/21/24.
//

#include "ft_nmap.h"

typedef struct {
  uint32_t src_addr;
  uint32_t dest_addr;
  uint8_t pholder;
  uint8_t protocol;
  uint16_t tcp_len;
} TCP_PseudoHeader;

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

uint16_t tcp_checksum(const void* vdata, const size_t length, const struct in_addr src_addr,
                      const struct in_addr dest_addr) {
  // Create the pseudo header
  TCP_PseudoHeader psh = {0};
  uint8_t pseudogram[1024] = {0};
  psh.src_addr = src_addr.s_addr;
  psh.dest_addr = dest_addr.s_addr;
  psh.pholder = 0;
  psh.protocol = IPPROTO_TCP;
  psh.tcp_len = htons(length);

  // Calculate the size for the pseudo header + TCP header
  memcpy(pseudogram, (char*)&psh, sizeof(TCP_PseudoHeader));
  memcpy(pseudogram + sizeof(TCP_PseudoHeader), vdata, length);
  const uint16_t chcksm = checksum((uint16_t*)pseudogram, sizeof(TCP_PseudoHeader) + length);
  return chcksm;
}

