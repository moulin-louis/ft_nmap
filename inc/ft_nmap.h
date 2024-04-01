//
// Created by loumouli on 3/20/24.
//

#ifndef FT_NMAP_H
#define FT_NMAP_H

// standard headers
#include <argp.h>
#include <ifaddrs.h>
#include <math.h>
#include <netinet/ether.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <pcap.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
// ----------------
#define ARRAY_IMPL
#define ARRAY_USE_IMPL
// library headers
#include <array.h>

// local headers
#include "utils.h"
// -------------

#define NMAP_SUCCESS 0
#define NMAP_FAILURE 1
typedef uint32_t NMAP_ScanType;
typedef enum e_nmap_port_status NMAP_PortStatus;
typedef struct s_nmap_options NMAP_Options;
typedef struct s_nmap_worker_options NMAP_WorkerOptions;
typedef struct s_nmap_worker_data NMAP_WorkerData;

#define NMAP_SCAN_NONE 0b000000 // DIFFERENT THAT SCAN_NULL x)
#define NMAP_SCAN_SYN 0b000001
#define NMAP_SCAN_NULL 0b000010
#define NMAP_SCAN_FIN 0b000100
#define NMAP_SCAN_XMAS 0b001000
#define NMAP_SCAN_ACK 0b010000
#define NMAP_SCAN_UDP 0b100000
#define NMAP_SCAN_ALL 0b111111

enum e_nmap_option_key {
  NMAP_KEY_IP = 'i',
  NMAP_KEY_FILE = 'f',
  NMAP_KEY_SCAN = 'S',
  NMAP_KEY_SPEEDUP = 's',
  NMAP_KEY_PORTS = 'p',
};

enum e_nmap_port_status {
  NMAP_UNKNOWN = 0, // == 0
  NMAP_OPEN = 1 << 1, // == 2
  NMAP_CLOSE = 1 << 2, // == 4
  NMAP_FILTERED = 1 << 3, // == 8
  NMAP_UNFILTERED = 1 << 4, // == 16
};

struct s_nmap_options {
  uint32_t scan;
  uint8_t speedup;
  Array* ips; // Array <in_addr_t>
  Array* ports; // Array<uint16_t>
};

struct s_nmap_worker_options {
  uint32_t scan;
  const Array* ips; // Array<in_addr_t>
  Array* ports; // Array<uint16_t>
};

struct s_nmap_worker_data {
  pthread_t thread;
  NMAP_WorkerOptions options;
  void* result;
};

#include "t_host.h"
#include "ultra_scan.h"

// options.c
void NMAP_printOptions(const NMAP_Options* options);
void NMAP_printWorkerOptions(const NMAP_WorkerOptions* options);
// ---------

// parser.c
NMAP_Options* NMAP_parseArgs(int argc, char** argv);
// --------

// scan_types.c
uint32_t NMAP_getScanNumber(const char* name);
// ------------

// worker.c
int NMAP_spawnWorkers(const NMAP_Options* options);
// --------


// Engine function
int64_t ultra_scan(const Array* ips, const Array* ports, NMAP_ScanType scanType, Array* thread_result);

// Packet I/O

uint64_t send_packet(int sck, const uint8_t* packet, uint64_t size_packet, int32_t flag, const struct sockaddr* dest);

// TCP SYN  Function

int32_t tcp_syn_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src);

NMAP_PortStatus tcp_syn_analysis(const struct iphdr* ip_hdr, const void* ip_payload);

// TCP ACK Function

int32_t tcp_ack_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src);

NMAP_PortStatus tcp_ack_analysis(const struct iphdr* ip_hdr, const void* ip_payload);

// TCP NULL Function

int32_t tcp_null_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src);

NMAP_PortStatus tcp_null_analysis(const struct iphdr* ip_hdr, const void* ip_payload);

// TCP FIN Function

int32_t tcp_fin_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src);

NMAP_PortStatus tcp_fin_analysis(const struct iphdr* ip_hdr, const void* ip_payload);

// TCP XMAS Function

int32_t tcp_xmas_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src);

NMAP_PortStatus tcp_xmas_analysis(const struct iphdr* ip_hdr, const void* ip_payload);


// Utils
char* port_status_to_string(NMAP_PortStatus status);
#endif
