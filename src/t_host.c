//
// Created by loumouli on 3/27/24.
//

#include "ft_nmap.h"

static bool ArrayFn_portIsPending(unused const Array* arr, unused size_t i, const void* value, unused void* param) {
  const t_port* const port = value;
  return port->probeStatus == PROBE_PENDING;
}

bool host_hasPortPendingLeft(const t_host* host) { return array_anyIf(host->ports, ArrayFn_portIsPending, NULL); }

static bool ArrayFn_portIsLeft(unused const Array* arr, unused size_t i, const void* value, unused void* param) {
  const t_port* const port = value;
  return port->probeStatus == PROBE_PENDING || port->probeStatus == PROBE_SENT;
}

bool host_hasPortLeft(const t_host* host) { return array_anyIf(host->ports, ArrayFn_portIsLeft, NULL); }

t_port* host_nextIncPort(t_host* host) {
  t_port* result = array_get(host->ports, host->idx_ports);
  host->idx_ports += 1;
  if (host->idx_ports >= array_size(host->ports))
    host->idx_ports = 0;
  return result;
}
