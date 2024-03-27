//
// Created by loumouli on 3/27/24.
//

#ifndef t_host_H
#define t_host_H

#include "ft_nmap.h"

typedef enum {
  PROBE_PENDING = 0,
  PROBE_SENT = 1 << 0,
  PROBE_RECV = 1 << 1,
  PROBE_TIMEOUT = 1 << 2
} ProbeStatus ;

typedef struct {
  uint16_t port;
  NMAP_PortStatus result;
  ProbeStatus probeStatus;
  struct timeval sendTime;
  struct timeval recvTime;
  uint64_t nprobes_sent;
} t_port;

/**
 * @brief Structure to store all the information needed for ultra_scan engine.
 * @param {struct in_addr} ip - IP address of the host.
 * @param {Array<t_port>} incompletePorts - Vector of ports to scan.
 * @param {t_port*} nextI - Next port to scan.
 */
typedef struct {
  struct in_addr ip;
  uint64_t idx_ports;
  Array* ports;
  t_port* nextIter;
} t_host;

bool host_hasPortLeft(const t_host* host);
t_port* host_nextIncPort(t_host* host);

#endif // t_host_H
