//
// Created by loumouli on 3/20/24.
//

#ifndef FT_NMAP_H
#define FT_NMAP_H

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
  uint32_t src_addr;
  uint32_t dest_addr;
  uint8_t pholder;
  uint8_t protocol;
  uint16_t tcp_len;
} TCP_PseudoHeader;

#endif // FT_NMAP_H
