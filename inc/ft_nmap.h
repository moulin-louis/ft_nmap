//
// Created by loumouli on 3/20/24.
//

#ifndef FT_NMAP_H
#define FT_NMAP_H

// standard headers
#include <argp.h>
#include <arpa/inet.h>
#include <error.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// ----------------

// local headers
#include "utils.h"
// -------------


#define NMAP_SUCCESS 0
#define NMAP_FAILURE 1


typedef struct s_nmap_options NMAP_Options;
typedef struct s_nmap_worker_options NMAP_WorkerOptions;
typedef enum e_nmap_scan_type NMAP_ScanType;
typedef enum e_nmap_option_key NMAP_OptionKey;
typedef enum e_nmap_port_status NMAP_PortStatus;

enum e_nmap_scan_type {
  NMAP_SCAN_INVALID = -1,
  NMAP_SCAN_SYN,
  NMAP_SCAN_NULL,
  NMAP_SCAN_FIN,
  NMAP_SCAN_XMAS,
  NMAP_SCAN_ACK,
  NMAP_SCAN_UDP,
};

enum e_nmap_option_key {
  NMAP_KEY_IP = 'i',
  NMAP_KEY_FILE = 'f',
  NMAP_KEY_SCAN = 'S',
  NMAP_KEY_SPEEDUP = 's',
  NMAP_KEY_PORTS = 'p',
};

enum e_nmap_port_status {
  OPEN = 1 << 0,
  CLOSE = 1 << 1,
  FILTERED = 1 << 2,
  UNFILTERED = 1 << 3,
};

struct s_nmap_options {
  in_addr_t ip;
  NMAP_ScanType scan;
  uint8_t speedup;
  uint16_t nPorts;
  uint16_t ports[UINT16_MAX];
};

struct s_nmap_worker_options {
  in_addr_t ip;
  NMAP_ScanType scan;
  uint16_t nPorts;
  uint16_t ports[UINT16_MAX];
};

// parser.c
NMAP_Options NMAP_parseArgs(int argc, char** argv);
// --------

// scan_types.c
const char* NMAP_getScanName(NMAP_ScanType scan);
NMAP_ScanType NMAP_getScanNumber(const char* name);
// ------------

// worker.c
int NMAP_spawnWorkers(const NMAP_Options* options);
// --------


// Packet I/O

uint64_t recv_packet(int sck, uint8_t* packet, uint64_t size_packet, int32_t flag, struct sockaddr* sender);

uint64_t send_packet(int sck, const uint8_t* packet, uint64_t size_packet, int32_t flag, const struct sockaddr* dest);

// TCP Function
uint16_t checksum(uint16_t* buffer, int size);

uint16_t tcp_checksum(const void* vdata, size_t length, struct in_addr src_addr, struct in_addr dest_addr);

// TCP SYN  Function

void tcp_syn_craft_payload(struct tcphdr* tcp_hdr, uint16_t port);

NMAP_PortStatus tcp_syn_analysis(const struct iphdr* ip_hdr, const uint8_t* ip_packet);

int64_t tcp_syn_cleanup(int sck, uint8_t* packet, uint64_t size_packet, int32_t flag, const struct sockaddr* dest);

// Utils
void ft_hexdump(const void* data, uint64_t nbytes, uint64_t row);

#endif
