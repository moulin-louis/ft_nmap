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

typedef enum {
  OPEN = 1 << 0,
  CLOSE = 1 << 1,
  FILTERED = 1 << 2,
  UNFILTERED = 1 << 3,
} NMAP_PortStatus;

// Packet I/O

uint64_t recv_packet(int sck, uint8_t* packet, uint64_t size_packet, int32_t flag, struct sockaddr* sender);

uint64_t send_packet(int sck, const uint8_t* packet, uint64_t size_packet, int32_t flag, const struct sockaddr* dest);

// TCP Function
uint16_t checksum(uint16_t* buffer, int size);

uint16_t tcp_checksum(const void* vdata, size_t length, struct in_addr src_addr, struct in_addr dest_addr);

// TCP SYN  Function

void tcp_syn_craft_payload(struct tcphdr* tcp_hdr, uint16_t port);

NMAP_PortStatus tcp_syn_analysis(const struct tcphdr* tcp_hdr);

int64_t tcp_syn_cleanup(int sck, uint8_t* packet, uint64_t size_packet, int32_t flag, const struct sockaddr* dest);

// Utils
void ft_hexdump(const void* data, uint64_t nbytes, uint64_t row);

#endif // FT_NMAP_H
