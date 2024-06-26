//
// Created by loumouli on 3/27/24.
//

#ifndef t_host_H
#define t_host_H

#include "ft_nmap.h"

typedef enum { PROBE_PENDING = 0, PROBE_SENT = 1 << 0, PROBE_RECV = 1 << 1, PROBE_TIMEOUT = 1 << 2 } NMAP_ProbeStatus;

/**
 * @brief Structure storing all the info needed for a port.
 * @param {uint16_t} port - Port number.
 * @param {NMAP_PortStatus} result - Result of the scan.
 * @param {ProbeStatus} probeStatus - Status of the current probe.
 * @param {struct timeval} sendTime - Time when the probe was sent.
 * @param {struct timeval} recvTime - Time when the probe was received.
 * @param {uint64_t} nprobes_sent - Number of probes sent.
 * @param {uint64_t} _padding - Padding to align the structure on a 64 bits boundary.
 */
typedef struct s_port {
  uint16_t port;
  NMAP_PortStatus result;
  NMAP_ProbeStatus probeStatus;
  struct timeval sendTime;
  struct timeval recvTime;
  uint32_t nprobes_sent;
  unused uint8_t _padding[18];
} __attribute__((packed)) t_port;

/**
 * @brief Structure to store all the information needed for ultra_scan engine.
 * @param {struct in_addr} ip - IP address of the host.
 * @param {Array<t_port>} incompletePorts - Vector of ports to scan.
 * @param {uint16_t} idx_ports - Index of the next port to scan.
 * @param {bool} done - True if all the ports have been scanned.
 * @param {uint8_t} _padding - Padding to align the struc on a 32 bits boundary.
 */
typedef struct s_host {
  struct in_addr ip;
  Array* ports;
  uint16_t idx_ports;
  uint16_t done;
  unused uint8_t _padding[16];
} __attribute__((packed)) t_host;

bool host_hasPortPendingLeft(const t_host* host);
bool host_hasPortLeft(const t_host* host);
t_port* host_nextIncPort(t_host* host);

#endif // t_host_H
