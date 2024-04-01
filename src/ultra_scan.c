//
// Created by loumouli on 3/27/24.
//

#include "ft_nmap.h"

static bool us_allHostDone(NMAP_UltraScan* us) {
  for (uint64_t i = 0; i < array_size(us->hosts); ++i) {
    t_host* host = array_get(us->hosts, i);
    if (host->done == false)
      return false;
  }
  return true;
}

/**
 * @brief update the SRTT of the NMAP_UltraScan structure based on the probe received.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan struc
 * @param port {const t_port*} - Probe recieved to update the SRTT
 */
static void us_updateSRTT(NMAP_UltraScan* us, const t_port* port) {
  us->srtt = us->srtt + (TIMEVAL_TO_MICROSC(port->recvTime) - TIMEVAL_TO_MICROSC(port->sendTime) - us->srtt) / 8;
}

/**
 * @brief update the RTTVAR of the NMAP_UltraScan structure based on the probe received.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan struc
 * @param port {const t_port*} - Probe recieved to update the RTTVAR
 */
static void us_updateRTTVAR(NMAP_UltraScan* us, const t_port* port) {
  us->rttvar = us->rttvar +
    (fabsl(TIMEVAL_TO_MICROSC(port->recvTime) - TIMEVAL_TO_MICROSC(port->sendTime) - us->srtt) - us->rttvar) / 4;
}

/**
 * @brief update the timeout of the NMAP_UltraScan structure based on the SRTT and RTTVAR.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param port {const t_port*} - Probe received to update the timeout.
 */
static void us_updateTimeout(NMAP_UltraScan* us, const t_port* port) {
  us_updateSRTT(us, port);
  us_updateRTTVAR(us, port);
  us->timeout = us->srtt + us->rttvar * 4;
  if (us->timeout < us->minTimeout)
    us->timeout = us->minTimeout;
  else if (us->timeout > us->maxTimeout)
    us->timeout = us->maxTimeout;
}

/**
 * @brief init NMAP_UltraScan structure to default value.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure to initialize.
 */
static void us_default_init(NMAP_UltraScan* us) {
  us->srtt = 0; // in micro seconds
  us->rttvar = 0; // in micro seconds
  us->timeout = 1'000'000; // in micro seconds (1s/1000ms)
  us->maxTimeout = 10'000'000; // in micro seconds (10s/10.000ms)
  us->minTimeout = 100'000; // in micro seconds (0.1s/100ms)
  us->maxRetries = 10;
}

static int ArrayFn_mapPortNumToHostPort(const Array* arr, size_t i, void* dst, const void* src, void* param) {
  memset(dst, 0, sizeof(t_port));
  (void)arr, (void)param, (void)i;
  ((t_port*)dst)->port = *(uint16_t*)src;
  return 0;
}

static int ArrayFn_mapIpToHost(unused const Array* arr, unused size_t i, void* dst, const void* src, void* param) {
  t_host* const host = dst;

  memset(host, 0, sizeof(t_host));
  host->ip = *(struct in_addr*)src;
  host->ports = array_cMap(param, sizeof(t_port), NULL, ArrayFn_mapPortNumToHostPort, NULL);
  if (host->ports == NULL)
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
static int64_t us_createHost(NMAP_UltraScan* us, const Array* ips, const Array* ports) {
  us->hosts = array_cMap(ips, sizeof(t_host), NULL, ArrayFn_mapIpToHost, (void*)ports);
  if (us->hosts == NULL)
    return 1;
  return 0;
}

/**
 * @brief init_sniffer and apply filter to the sniffer.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
static int64_t init_sniffer(NMAP_UltraScan* us) {
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
    const t_host* host = array_cGet(us->hosts, targetno);
    const char* str = inet_ntoa(host->ip);
    strcat(dst_hosts, str);
  }
  us->handle = pcap_open_live(devs->name, 10000, 1, 1, errbuf);
  if (us->handle == NULL) {
    pcap_freealldevs(devs);
    fprintf(stderr, "pcap_open_live: %s\n", errbuf);
    return 1;
  }
  strcat(pcap_filter, "dst host ");
  strcat(pcap_filter, inet_ntoa(get_interface_ip(devs->name)));
  pcap_freealldevs(devs);
  strcat(pcap_filter, " and (icmp or (tcp and (");
  strcat(pcap_filter, dst_hosts);
  strcat(pcap_filter, ")))");
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
  //  printf("filter = [%s]\n", pcap_filter);
  free(fp.bf_insns);
  return 0;
}

/**
 * @brief return the next host to scan and increment the nextIter.
 * @param us {NMAP_UltraScan*} UltraScan structure.
 * @return {t_host*} - Next host to scan.
 */
static t_host* us_nextHost(NMAP_UltraScan* us) {
  t_host* result = array_get(us->hosts, us->idxNextHosts);
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
static int64_t sendNextScanProbe(NMAP_UltraScan* us, t_host* host) {
  t_port* port = host_nextIncPort(host);
  us->packet_sent += 1;
  //  printf("sending one probe\n");
  switch (us->scanType) {
  case NMAP_SCAN_SYN:
    if (tcp_syn_send_probe(us, port, host->ip, us->inter_ip))
      return 1;
    break;
  case NMAP_SCAN_ACK:
    if (tcp_ack_send_probe(us, port, host->ip, us->inter_ip))
      return 1;
    break;
  case NMAP_SCAN_NULL:
    if (tcp_null_send_probe(us, port, host->ip, us->inter_ip))
      return 1;
    break;
  case NMAP_SCAN_FIN:
    if (tcp_fin_send_probe(us, port, host->ip, us->inter_ip))
      return 1;
    break;
  case NMAP_SCAN_XMAS:
    if (tcp_xmas_send_probe(us, port, host->ip, us->inter_ip))
      return 1;
    break;
  default:
    fprintf(stderr, "Unsuported scan type\n");
    fprintf(stderr, "got scan = %d\n", us->scanType);
    exit(4);
  }
  port->probeStatus = PROBE_SENT;
  port->nprobes_sent += 1;
  return 0;
}

/**
 * @brief send probe to any needed target in targets.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
static int64_t doAnyNewProbe(NMAP_UltraScan* us) {
  t_host* host = us_nextHost(us);
  const t_host* unableToSend = NULL;
  while (host != NULL && host != unableToSend) {
    if (host_hasPortPendingLeft(host)) {
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

/**
 * @brief wait for the fd of the pcap handle to be ready.
 * @param p {pcap_t*} - pcap handler to use
 * @param to_usec {long} - timeout to wait for in microseconds.
 * @return {int64_t} - 0 if timeout, -1 on error, > 0 if fd is ready.
 */
static int64_t pcap_poll(pcap_t* p, const int64_t to_usec) {
  int fd;
  if ((fd = pcap_get_selectable_fd(p)) == -1)
    return -1;

  struct pollfd fds = {.fd = fd, .events = POLLIN, .revents = 0};
  errno = 0;
  return poll(&fds, 1, (int32_t)(to_usec / 1000)); // we convert to milliseconds
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
static int64_t read_reply_pcap(pcap_t* handle, const int64_t to_usec, const uint8_t** packet, struct pcap_pkthdr** head,
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
    if (pcap_status == 1 && *packet != NULL) // if its a good packet
      break;
    if (pcap_status == 0 || *packet == NULL) {
      gettimeofday(&tv_end, NULL);
      if (TIMEVAL_SUBTRACT(tv_end, tv_start) >= to_usec)
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
static bool get_pcap_result(NMAP_UltraScan* us, const struct timeval* stime) {
  struct timeval rcvdtime;
  struct pcap_pkthdr* head;
  const uint8_t* packet;

  gettimeofday(&us->now, NULL);
  long to_usec = TIMEVAL_SUBTRACT(*stime, us->now);
  if (to_usec < 2000)
    to_usec = 2000;

  if (read_reply_pcap(us->handle, to_usec, &packet, &head, &rcvdtime))
    return false;
  struct iphdr* iphdr = (struct iphdr*)(packet + sizeof(struct ether_header));
  gettimeofday(&us->now, NULL);
  if (iphdr == NULL || TIMEVAL_SUBTRACT(us->now, *stime) > us->timeout)
    return false;
  us->packet_recv += 1;
  const void* payload = (void*)(packet + sizeof(struct ether_header) + sizeof(struct iphdr));
  NMAP_PortStatus result = NMAP_UNKNOWN;
  switch (us->scanType) {
  case NMAP_SCAN_SYN:
    result = tcp_syn_analysis(iphdr, payload);
    break;
  case NMAP_SCAN_ACK:
    result = tcp_ack_analysis(iphdr, payload);
    break;
  case NMAP_SCAN_NULL:
    result = tcp_null_analysis(iphdr, payload);
    break;
  case NMAP_SCAN_FIN:
    result = tcp_fin_analysis(iphdr, payload);
    break;
  case NMAP_SCAN_XMAS:
    result = tcp_xmas_analysis(iphdr, payload);
    break;
  }
  if (result == NMAP_UNKNOWN) {
    printf("unknown state\n");
    return false;
  }
  uint16_t src_port = 0;
  if (iphdr->protocol == IPPROTO_TCP) {
    const struct tcphdr* tcp_tmp = (struct tcphdr*)payload;
    src_port = ntohs(tcp_tmp->source);
  }
  if (iphdr->protocol == IPPROTO_ICMP) {
    struct icmphdr* icmp_hdr = (struct icmphdr*)payload;
    if (icmp_hdr->type == ICMP_DEST_UNREACH) {
      struct iphdr* original_ip_hdr = (struct iphdr*)((unsigned char*)icmp_hdr + sizeof(struct icmphdr));
      int original_ip_hdr_len = (original_ip_hdr->ihl & 0x0f) * 4;
      if (original_ip_hdr_len < 20) {
        printf("Invalid IP header length.\n");
        return false;
      }
      struct tcphdr* original_tcp_hdr = (struct tcphdr*)((unsigned char*)original_ip_hdr + original_ip_hdr_len);
      // we only have access to the first 8 bytes tcp_header so -> src_port + dest_port + seq_nbr
      src_port = ntohs(original_tcp_hdr->dest);
    }
  }

  const struct in_addr ip_src = *(struct in_addr*)&iphdr->saddr;
  //  printf("recv packet from %s\n", inet_ntoa(ip_src));
  for (uint64_t i = 0; i < array_size(us->hosts); ++i) {
    t_host* host = array_get(us->hosts, i);
    if (memcmp(&host->ip.s_addr, &ip_src, sizeof(ip_src)) != 0)
      continue;
    for (uint64_t j = 0; j < array_size(host->ports); ++j) {
      t_port* port = array_get(host->ports, j);
      if (port->port != src_port)
        continue;
      port->result = result;
      port->probeStatus = PROBE_RECV;
      port->recvTime = rcvdtime;
      us_updateTimeout(us, port);
      break;
    }
  }
  return true;
}

/**
 * @brief recv and process packet until there is no more packet to process or timeout.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 */
static void waitForResponses(NMAP_UltraScan* us) {
  bool gotone = true;
  struct timeval stime = {0};
  gettimeofday(&stime, NULL);
  while (gotone) {
    gettimeofday(&us->now, NULL);
    gotone = get_pcap_result(us, &stime);
  }
}

/**
 * @brief - Handle timeout for sent probe and check the number of retries
 * @param us {NMAP_Ultrascan*} - UltraScan structure
 */
static void doAnyOustandingRetransmit(NMAP_UltraScan* us) {
  gettimeofday(&us->now, NULL);
  for (uint64_t i = 0; i < array_size(us->hosts); ++i) {
    const t_host* host = array_get(us->hosts, i);
    for (uint64_t j = 0; j < array_size(host->ports); ++j) {
      t_port* port = array_get(host->ports, j);
      if (port->probeStatus == PROBE_SENT) {
        if (TIMEVAL_SUBTRACT(us->now, port->sendTime) > us->timeout) {
          if (port->nprobes_sent < us->maxRetries) {
            //            printf("port %u: retrying\n", port->port);
            port->probeStatus = PROBE_PENDING;
          }
          else {
            //            printf("port %u: timeout\n", port->port);
            port->result = NMAP_FILTERED;
            port->probeStatus = PROBE_TIMEOUT;
          }
        }
      }
    }
  }
}

/**
 * @brief ultra_scan
 * @param ips {Array<in_addr>} - Vector of targets to scan.
 * @param ports {Array<uint16_t>} - Vector of ports to scan.
 * @param scanType {NMAP_ScanType} - Type of scan to perform.
 * @param thread_result {Array<Array<t_host>} - Actual result of all the scan
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t ultra_scan(const Array* ips, const Array* ports, const NMAP_ScanType scanType, Array* thread_result) {
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
    printf("init sniffer failed\n");
    return 1;
  }
  while (us_allHostDone(&us) == false) {
    doAnyOustandingRetransmit(&us);
    if (doAnyNewProbe(&us)) {
      array_destroy(us.hosts);
      return 1;
    }
    waitForResponses(&us);
    for (uint64_t i = 0; i < array_size(us.hosts); ++i) {
      t_host* host = array_get(us.hosts, i);
      if (host_hasPortLeft(host) == false) {
        host->done = true;
      }
    }
  }
  printf("%ld packet sent\n", us.packet_sent);
  printf("%ld packet recv\n", us.packet_recv);
  pcap_close(us.handle);
  close(us.sock);
  array_pushBack(thread_result, &us.hosts, 1);
  return 0;
}