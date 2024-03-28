//
// Created by loumouli on 3/27/24.
//

#include "ft_nmap.h"


bool host_hasPortPendingLeft(const t_host* host) {
  for (uint64_t i = 0; i < array_size(host->ports); ++i) {
    const t_port* port = array_get(host->ports, i);
    if (port->probeStatus == PROBE_PENDING)
      return true;
  }
  return false;
}

bool host_hasPortLeft(const t_host* host) {
  for (uint64_t i = 0; i < array_size(host->ports); ++i) {
    const t_port* port = array_get(host->ports, i);
    if (port->probeStatus == PROBE_PENDING || port->probeStatus == PROBE_SENT)
      return true;
  }
  return false;
}

t_port* host_nextIncPort(t_host* host) {
  t_port* result = array_get(host->ports, host->idx_ports);
  host->idx_ports += 1;
  if (host->idx_ports >= array_size(host->ports))
    host->idx_ports = 0;
  return result;
}