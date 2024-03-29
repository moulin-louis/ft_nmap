//
// Created by loumouli on 3/27/24.
//

#include "ft_nmap.h"

/**
 * @brief init NMAP_UltraScan structure to default value.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure to initialize.
 */
void us_default_init(NMAP_UltraScan* us) {
  us->srtt = 0;
  us->rttvar = 0;
  us->timeout = 1'000'000;
  us->maxTimeout = 10'000'000;
  us->minTimeout = 100'000;
  us->maxRetries = 10;
}

/**
 * @brief update the SRTT of the NMAP_UltraScan structure based on the probe received.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan struc
 * @param port {const t_port*} - Probe recieved to update the SRTT
 */
void us_updateSRTT(NMAP_UltraScan* us, const t_port* port) {
  us->srtt = us->srtt + (TIMEVAL_TO_MICROSC(port->recvTime) - TIMEVAL_TO_MICROSC(port->sendTime) - us->srtt) / 8;
}

/**
 * @brief update the RTTVAR of the NMAP_UltraScan structure based on the probe received.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan struc
 * @param port {const t_port*} - Probe recieved to update the RTTVAR
 */
void us_updateRTTVAR(NMAP_UltraScan* us, const t_port* port) {
  us->rttvar = us->rttvar +
    (fabsl(TIMEVAL_TO_MICROSC(port->recvTime) - TIMEVAL_TO_MICROSC(port->sendTime) - us->srtt) - us->rttvar) / 4;
}

/**
 * @brief update the timeout of the NMAP_UltraScan structure based on the SRTT and RTTVAR.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param port {const t_port*} - Probe received to update the timeout.
 */
void us_updateTimeout(NMAP_UltraScan* us, const t_port* port) {
  us_updateSRTT(us, port);
  us_updateRTTVAR(us, port);
  us->timeout = us->srtt + us->rttvar * 4;
  if (us->timeout < us->minTimeout)
    us->timeout = us->minTimeout;
  if (us->timeout > us->maxTimeout)
    us->timeout = us->maxTimeout;
}

/**
 * @brief Check if there is any host with pending ports.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @return {bool} - true if there is a host with pending ports, false otherwise.
 */
bool old_us_hasIncompleteHosts(NMAP_UltraScan* us) {
  for (uint64_t i = 0; i < array_size(us->hosts); ++i) {
    const t_host* host = *(t_host**)array_get(us->hosts, i);
    if (old_host_hasPortLeft(host))
      return true;
  }
  return false;
}

static void ArrayFn_hostsDestructor(Array* arr, void* data, size_t n) {
  for (size_t i = 0; i < n; ++i)
    array_destroy(((t_host*)data)[i].ports);
}

static int ArrayFn_mapPortNumToHostPort(const Array* arr, size_t i, void* dst, const void* src, void* param) {
  ((t_port*)dst)->port = *(uint16_t*)src;
  return 0;
}

static int ArrayFn_mapIpToHost(const Array* arr, size_t i, void* dst, const void* src, void* param) {
  const Array* const ports = param;
  t_host* const host = dst;

  host->ip = *(struct in_addr*)src;
  host->ports = array_cMap(ports, sizeof(t_port), NULL, ArrayFn_mapPortNumToHostPort, NULL);
  if (!host->ports)
    return 1;
  return 0;
}

/**
 * @brief create host vectors based on input ips and ports.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure to initialize.
 * @param ips  {Array<struct addr_in>} - Vector of IP addresses to filter.
 * @param ports {Array<uint16_t>} - Vector of ports to scan.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t us_createHost(NMAP_UltraScan* us, const Array* ips, const Array* ports) {
  const ArrayFactory us_hostsFactory = {
    .destructor = ArrayFn_hostsDestructor,
  };

  us->hosts = array_cMap(ips, sizeof(t_host), &us_hostsFactory, ArrayFn_mapIpToHost, ports);
  if (!us->hosts)
    return 1;
  return 0;
}

int64_t old_us_createHost(NMAP_UltraScan* us, const Array* ips, const Array* ports) {
  us->hosts = array(sizeof(t_host*), 0, 0, NULL, NULL);
  if (us->hosts == NULL)
    return 1;
  for (uint64_t i = 0; i < array_size(ips); ++i) {
    t_host* host = calloc(1, sizeof(t_host));
    if (host == NULL) {
      perror("calloc/us_createHost/host");
      array_destroy(us->hosts);
      return 1;
    }
    printf("host ptr = %p\n", host);
    host->ip = *(struct in_addr*)array_cGet(ips, i);
    host->ports = array(sizeof(t_port), 0, 0, NULL, NULL);
    if (host->ports == NULL) {
      perror("calloc/us_createHost/ports");
      array_destroy(us->hosts);
      return 1;
    }
    for (uint64_t j = 0; j < array_size(ports); ++j) {
      t_port port = {0};
      port.port = *(uint16_t*)array_cGet(ports, j);
      array_pushFront(host->ports, &port, 1);
    }
    array_pushFront(us->hosts, &host, 1);
  }
  return 0;
}

/**
 * @brief return the next host to scan and increment the nextIter.
 * @param us {NMAP_UltraScan*} UltraScan structure.
 * @return {t_host*} - Next host to scan.
 */
t_host* us_nextHost(NMAP_UltraScan* us) {
  t_host* result = array_get(us->hosts, us->idxNextHosts);
  us->idxNextHosts++;
  if (us->idxNextHosts >= array_size(us->hosts))
    us->idxNextHosts = 0;
  return result;
}

t_host** us_old_nextHost(NMAP_UltraScan* us) {
  t_host** result = array_get(us->hosts, us->idxNextHosts);
  us->idxNextHosts++;
  if (us->idxNextHosts >= array_size(us->hosts))
    us->idxNextHosts = 0;
  return result;
}

/**
 * @brief send a probe to the next port of a given host
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param host {t_host*} - Host to send the probe to.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t sendNextScanProbe(const NMAP_UltraScan* us, t_host* host) {
  t_port* port = host_nextIncPort(host);
  switch (us->scanType) {
  case NMAP_SCAN_SYN:
    if (tcp_syn_send_probe(us, port, host->ip, us->inter_ip))
      return 1;
    break;
  default: {
  }
  }
  port->probeStatus = PROBE_SENT;
  port->nprobes_sent += 1;
  return 0;
}

static bool ArrayFn_portIsPending(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)param;
  const t_port* const port = value;
  return port->probeStatus == PROBE_PENDING;
}

/**
 * @brief send probe to any needed target in targets.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t doAnyNewProbe(NMAP_UltraScan* us) {
  t_host* host = us_nextHost(us);
  const t_host* unableToSend = NULL;
  while (host != NULL && host != unableToSend) {
    if (array_anyIf(host->ports, ArrayFn_portIsPending, NULL)) {
      if (sendNextScanProbe(us, host))
        return 1;
      unableToSend = NULL;
    }
    else if (unableToSend == NULL)
      unableToSend = host;
    host = us_nextHost(us);
  }
  return 0;
}

int64_t old_doAnyNewProbe(NMAP_UltraScan* us) {
  t_host* host = *us_old_nextHost(us);
  const t_host* unableToSend = NULL;
  while (host != NULL && host != unableToSend) {
    if (old_host_hasPortPendingLeft(host)) {
      if (sendNextScanProbe(us, host))
        return 1;
      unableToSend = NULL;
    }
    else if (unableToSend == NULL)
      unableToSend = host;
    host = *us_old_nextHost(us);
  }
  return 0;
}

/**
 * @brief wait for the fd of the pcap handle to be ready.
 * @param p {pcap_t*} - pcap handler to use
 * @param to_usec {long} - timeout to wait for in microseconds.
 * @return {int64_t} - 0 if timeout, -1 on error, > 0 if fd is ready.
 */
int64_t pcap_poll(pcap_t* p, const int64_t to_usec) {
  int fd;
  if ((fd = pcap_get_selectable_fd(p)) == -1)
    return -1;

  struct pollfd fds = {.fd = fd, .events = POLLIN, .revents = 0};
  errno = 0;
  return poll(&fds, 1, to_usec / 1000); // we convert to milliseconds
}

/**
 * @brief read a packet from the pcap handle with a timeout.
 * @param handle {pcap_t*} - pcap handle
 * @param to_usec {long} - timeout in microseconds
 * @param packet {const uint8_t**} - pointer to a uint8_t pointer
 * @param head {struct pcap_pkthdr**} - pointer to a pcap header
 * @param rcvdtime  {struct timeval*} - time when the packet was received
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t read_reply_pcap(pcap_t* handle, const int64_t to_usec, const uint8_t** packet, struct pcap_pkthdr** head,
                        struct timeval* rcvdtime) {
  bool timeout = false;
  struct timeval tv_start, tv_end;
  gettimeofday(&tv_start, NULL);

  while (timeout == false) {
    int pcap_status = 0;
    *packet = NULL;
    if (pcap_poll(handle, to_usec) == 0)
      timeout = true;
    else
      pcap_status = pcap_next_ex(handle, head, packet);
    if (pcap_status == PCAP_ERROR)
      return 1;
    // printf("timeout == %d\n", timeout);
    if (pcap_status == 1 && *packet != NULL) // if its a good packet
      break;
    if (pcap_status == 0 || *packet == NULL) {
      gettimeofday(&tv_end, NULL);
      if (TIMEVAL_SUBTRACT(tv_end, tv_start) >= to_usec * 1000)
        timeout = 1;
    }
  }
  if (timeout)
    return 1;
  rcvdtime->tv_sec = (*head)->ts.tv_sec;
  rcvdtime->tv_usec = (*head)->ts.tv_usec;
  return 0;
}

/**
 * @brief grap a packet from the pcap handle and process it.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param stime {struct timeval*} - start time.
 * @return {bool} - true if there is a result, false otherwise.
 */
bool get_pcap_result(NMAP_UltraScan* us, const struct timeval* stime) {
  struct timeval rcvdtime;

  gettimeofday(&us->now, NULL);
  long to_usec = TIMEVAL_SUBTRACT(*stime, us->now);
  if (to_usec < 2000)
    to_usec = 2000;
  struct pcap_pkthdr* head;
  const uint8_t* packet;

  if (read_reply_pcap(us->handle, to_usec, &packet, &head, &rcvdtime))
    return NULL;
  struct iphdr* ip_tmp = (struct iphdr*)(packet + sizeof(struct ether_header));
  gettimeofday(&us->now, NULL);
  if (ip_tmp == NULL || TIMEVAL_SUBTRACT(us->now, *stime) > us->timeout)
    return false;
  const struct in_addr ip_src = *(struct in_addr*)&ip_tmp->saddr;
  const void* payload = (void*)(packet + sizeof(struct ether_header) + sizeof(struct iphdr));
  const struct tcphdr* tcp_tmp = (struct tcphdr*)payload;
  const NMAP_PortStatus result = tcp_syn_analysis(ip_tmp, payload);
  for (uint64_t i = 0; i < array_size(us->hosts); ++i) {
    t_host* host = *(t_host**)array_get(us->hosts, i);
    if (host->ip.s_addr != ip_src.s_addr)
      continue;
    for (uint64_t j = 0; j < array_size(host->ports); ++j) {
      t_port* port = array_get(host->ports, j);
      if (port->port != ntohs(tcp_tmp->source))
        continue;
      port->result = result;
      port->probeStatus = PROBE_RECV;
      port->recvTime = rcvdtime;
      us_updateTimeout(us, port);
      break;
    }
    // switch the host to done with every port have timeout or recv.
    if (host_hasPortLeft(host) == false)
      host->done = true;
    break;
  }
  return true;
}

/**
 * @brief recv and process packet until there is no more packet to process or timeout.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 */
void waitForResponses(NMAP_UltraScan* us) {
  bool gotone = true;
  struct timeval stime = {0};
  gettimeofday(&stime, NULL);
  while (gotone) {
    gettimeofday(&us->now, NULL);
    gotone = get_pcap_result(us, &stime);
  }
}

/**
 * @brief init_sniffer and apply filter to the sniffer.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t init_sniffer(NMAP_UltraScan* us) {
  const uint64_t nbrHosts = array_size(us->hosts);
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_if_t* devs;
  char dst_hosts[4096] = {0};
  char pcap_filter[4096] = {0};
  const bpf_u_int32 net = 0;
  struct bpf_program fp;

  if (pcap_findalldevs(&devs, errbuf) == -1) {
    fprintf(stderr, "pcap_findalldevs: %s\n", errbuf);
    return 1;
  }
  us->inter_ip = get_interface_ip(devs->name);
  for (uint64_t targetno = 0; targetno < nbrHosts; ++targetno) {
    if (targetno == 0)
      strcat(dst_hosts, "");
    else
      strcat(dst_hosts, " or ");
    strcat(dst_hosts, "src host ");
    const t_host* host = *(const t_host**)array_cGet(us->hosts, targetno);
    const char* str = inet_ntoa(host->ip);
    strcat(dst_hosts, str);
  }
  us->handle = pcap_open_live(devs->name, 10000, 1, 1, errbuf);
  if (us->handle == NULL) {
    pcap_freealldevs(devs);
    fprintf(stderr, "pcap_open_live: %s\n", errbuf);
    return 1;
  }
  if (pcap_setnonblock(us->handle, 1, errbuf) == -1) {
    pcap_close(us->handle);
    pcap_freealldevs(devs);
    fprintf(stderr, "pcap_setnonblock: %s\n", errbuf);
    return 1;
  }
  strcat(pcap_filter, "dst host ");
  strcat(pcap_filter, inet_ntoa(get_interface_ip(devs->name)));
  pcap_freealldevs(devs);
  strcat(pcap_filter, " and (icmp or (tcp and (");
  strcat(pcap_filter, dst_hosts);
  strcat(pcap_filter, ")))");
  printf("filter = [%s]\n", pcap_filter);
  if (pcap_compile(us->handle, &fp, pcap_filter, 0, net) == -1) {
    pcap_close(us->handle);
    fprintf(stdout, "Cant parse filter %s\n", pcap_geterr(us->handle));
    return 1;
  }
  if (pcap_setfilter(us->handle, &fp) == -1) {
    pcap_close(us->handle);
    fprintf(stderr, "Couldnt apply filter %s\n", pcap_geterr(us->handle));
    return 1;
  }
  free(fp.bf_insns);
  return 0;
}

static bool ArrayFn_portIsLeft(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)param;
  const t_port* const port = value;
  return port->probeStatus == PROBE_PENDING || port->probeStatus == PROBE_SENT;
}

static bool ArrayFn_hostHasPortLeft(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)param;
  const t_host* const host = value;
  return array_anyIf(host->ports, ArrayFn_portIsLeft, NULL);
}

/**
 * @brief ultra_scan
 * @param ips {Array<in_addr>} - Vector of targets to scan.
 * @param ports {Array<uint16_t>} - Vector of ports to scan.
 * @param scanType {NMAP_ScanType} - Type of scan to perform.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t ultra_scan(const Array* ips, const Array* ports, const NMAP_ScanType scanType) {
  NMAP_UltraScan us = {0};
  us.scanType = scanType;

  us_default_init(&us);
  us.sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (us.sock < 0) {
    perror("socket/ultra_scan");
    return 1;
  }
  if (us_createHost(&us, ips, ports)) {
    perror(array_strerror());
    return 1;
  }
  if (init_sniffer(&us)) {
    printf("init siffer failed\n");
    return 1;
  }
  while (array_anyIf(us.hosts, ArrayFn_hostHasPortLeft, NULL)) {
    // do outstanding retransmit
    if (doAnyNewProbe(&us)) {
      array_destroy(us.hosts);
      return 1;
    }
    waitForResponses(&us);
  }
  pcap_close(us.handle);
  close(us.sock);
  for (uint64_t i = 0; i < array_size(us.hosts); ++i) {
    t_host* host = array_get(us.hosts, i);
    for (uint64_t j = 0; j < array_size(host->ports); ++j) {
      t_port* port = array_get(host->ports, j);
      if (port->probeStatus == PROBE_SENT)
        port->result = FILTERED;
      if (port->result == OPEN)
        printf("port %u = %s\n", port->port, port_status_to_string(port->result));
    }
  }
  array_destroy(us.hosts);
  return 0;
}

int64_t old_ultra_scan(const Array* ips, const Array* ports, const NMAP_ScanType scanType) {
  NMAP_UltraScan us = {0};
  us.scanType = scanType;

  us_default_init(&us);
  us.sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (us.sock < 0) {
    perror("socket/ultra_scan");
    return 1;
  }
  old_us_createHost(&us, ips, ports);
  if (init_sniffer(&us)) {
    printf("init siffer failed\n");
    return 1;
  }
  while (old_us_hasIncompleteHosts(&us)) {
    // do outstanding retransmit
    if (old_doAnyNewProbe(&us))
      return 1;
    waitForResponses(&us);
  }
  pcap_close(us.handle);
  close(us.sock);
  for (uint64_t i = 0; i < array_size(us.hosts); ++i) {
    t_host* host = *(t_host**)array_get(us.hosts, i);
    for (uint64_t j = 0; j < array_size(host->ports); ++j) {
      t_port* port = array_get(host->ports, j);
      if (port->probeStatus == PROBE_SENT)
        port->result = FILTERED;
      if (port->result == OPEN)
        printf("port %u = %s\n", port->port, port_status_to_string(port->result));
    }
    array_destroy(host->ports);
    free(host);
  }
  array_destroy(us.hosts);
  return 0;
}
