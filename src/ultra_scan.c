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
  us->timeout = 1;
  us->maxTimeout = 10;
  us->minTimeout = 0.100;
  us->maxRetries = 10;
}

/**
 * @brief create incomplete host based on ips and ports.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure to initialize.
 * @param ips  {Array<struct addr_in>} - Vector of IP addresses to filter.
 * @param ports {Array<uint16_t>} - Vector of ports to scan.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t us_create_incHost(NMAP_UltraScan* us, const Array* ips, const Array* ports) {
  (void)us;
  (void)ips;
  (void)ports;
  us->incompleteHosts = array(sizeof(t_host*), 0, 0, NULL, NULL);
  for (uint64_t i = 0; i < array_size(ips); ++i) {
    const struct in_addr* ipAddr = array_cGet(ips, i);
    t_host* host = calloc(1, sizeof(t_host));
    if (host == NULL)
      return 1;
    host->ip = *ipAddr;
    host->ports = array_clone(ports);
    if (host->ports == NULL)
      return 1;
    host->nextIter = array_get(host->ports, 0);
    array_pushFront(us->incompleteHosts, &host, 1);
  }
  return 0;
}

/**
 * @brief return the next host to scan and increment the nextIter.
 * @param us {NMAP_UltraScan} UltraScan structure.
 * @return {t_host*} - Next host to scan.
 */
t_host* us_nextIncHost(NMAP_UltraScan* us) {
  t_host** result = (t_host**)us->nextIter;
  us->nextIter++;
  if ((void*)us->nextIter >= array_get(us->incompleteHosts, array_size(us->incompleteHosts)))
    us->nextIter = array_get(us->incompleteHosts, 0);
  return *result;
}

/**
 *
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param host {t_host*} - Host to send the probe to.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
static int64_t sendNextScanProbe(const NMAP_UltraScan* us, t_host* host) {
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

/**
 * @brief send probe to any needed target in targets.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t doAnyNewProbe(NMAP_UltraScan* us) {
  t_host* host = us_nextIncHost(us);
  const t_host* unableToSend = NULL;
  while (host != NULL && host != unableToSend) {
    if (host_hasPortLeft(host)) {
      if (sendNextScanProbe(us, host))
        return 1;
      unableToSend = NULL;
    }
    else if (unableToSend == NULL) {
      unableToSend = host;
    }
    host = us_nextIncHost(us);
  }
  return 0;
}

int pcap_select(pcap_t* p, struct timeval* timeout) {
  int fd;
  fd_set rfds;
  if ((fd = pcap_get_selectable_fd(p)) == -1) {
    printf("pcap error %s\n", pcap_geterr(p));
    return -1;
  }

  FD_ZERO(&rfds);
  return select(fd + 1, &rfds, NULL, NULL, timeout);
}
int datalink_offset(int datalink) {
  int offset = -1;
  /* NOTE: IF A NEW OFFSET EVER EXCEEDS THE CURRENT MAX (24), ADJUST
     MAX_LINK_HEADERSZ in libnetutil/netutil.h */
  switch (datalink) {
  case DLT_EN10MB:
    offset = ETHER_HDR_LEN;
    break;
  case DLT_IEEE802:
    offset = 22;
    break;
#ifdef __amigaos__
  case DLT_MIAMI:
    offset = 16;
    break;
#endif
#ifdef DLT_LOOP
  case DLT_LOOP:
#endif
  case DLT_NULL:
    offset = 4;
    break;
  case DLT_SLIP:
#ifdef DLT_SLIP_BSDOS
  case DLT_SLIP_BSDOS:
#endif
#if (FREEBSD || OPENBSD || NETBSD || BSDI || MACOSX)
    offset = 16;
#else
    offset = 24; /* Anyone use this??? */
#endif
    break;
  case DLT_PPP:
#ifdef DLT_PPP_BSDOS
  case DLT_PPP_BSDOS:
#endif
#ifdef DLT_PPP_SERIAL
  case DLT_PPP_SERIAL:
#endif
#ifdef DLT_PPP_ETHER
  case DLT_PPP_ETHER:
#endif
#if (FREEBSD || OPENBSD || NETBSD || BSDI || MACOSX)
    offset = 4;
#else
# ifdef SOLARIS
    offset = 8;
# else
    offset = 24; /* Anyone use this? */
# endif /* ifdef solaris */
#endif /* if freebsd || openbsd || netbsd || bsdi */
    break;
  case DLT_RAW:
    offset = 0;
    break;
  case DLT_FDDI:
    offset = 21;
    break;
#ifdef DLT_ENC
  case DLT_ENC:
    offset = 12;
    break;
#endif /* DLT_ENC */
#ifdef DLT_LINUX_SLL
  case DLT_LINUX_SLL:
    offset = 16;
    break;
#endif
#ifdef DLT_IPNET
  case DLT_IPNET:
    offset = 24;
    break;
#endif
  default:
    offset = -1;
    break;
  }
  return offset;
}

int read_reply_pcap(pcap_t* handle, long to_usec,
                    bool (*val_cb)(const uint8_t*, const struct pcap_pkthdr*, int, size_t), const uint8_t** p,
                    struct pcap_pkthdr** head, struct timeval* rcvdtime, int* datalink, size_t* offset) {
  int timedout = 0;
  int badcounter = 0;
  struct timeval tv_start, tv_end, to;

  to.tv_sec = to_usec / 1000000;
  to.tv_usec = to_usec % 1000000;
  if ((*datalink = pcap_datalink(handle)) < 0)
    return 1;
  const int ioffset = datalink_offset(*datalink);
  *offset = (unsigned int)ioffset;
  if (to_usec > 0)
    gettimeofday(&tv_start, NULL);
  do {
    int pcap_status = 0;
    *p = NULL;
    if (pcap_select(handle, &to) == 0)
      timedout = 1;
    else
      pcap_status = pcap_next_ex(handle, head, p);
    if (pcap_status == PCAP_ERROR)
      return 1;
    if (pcap_status == 1 && *p != NULL && val_cb(*p, *head, *datalink, *offset))
      break;
    if (pcap_status == 0 || *p == NULL) {
      if (to_usec == 0)
        timedout = 1;
      else if (to_usec > 0) {
        gettimeofday(&tv_end, NULL);
        if (TIMEVAL_SUBTRACT(tv_end, tv_start) >= to_usec)
          timedout = 1;
      }
    }
    else if (badcounter++ > 50)
      timedout = 1;
  }
  while (!timedout);
  if (timedout)
    return 0;
  if (rcvdtime) {
    rcvdtime->tv_sec = (*head)->ts.tv_sec;
    rcvdtime->tv_usec = (*head)->ts.tv_usec;
  }
  return 1;
}

static bool accept_ip(const unsigned char* p, const struct pcap_pkthdr* h, int datalink, size_t offset) {
  const struct ip* ip = NULL;
  (void)datalink;
  if (h->caplen < offset + sizeof(struct ip))
    return false;
  ip = (struct ip*)(p + offset);
  switch (ip->ip_v) {
  case 4:
  case 6:
    break;
  default:
    return false;
  }
  return true;
}

const uint8_t* readip_pcap(pcap_t* handle, unsigned int* len, long to_usec, struct timeval* rcvdtime,
                           struct link_header* linknfo, bool validate) {
  int datalink;
  size_t offset = 0;
  struct pcap_pkthdr* head;
  const uint8_t* p;

  if (linknfo)
    memset(linknfo, 0, sizeof(*linknfo));
  if (validate)
    read_reply_pcap(handle, to_usec, accept_ip, &p, &head, rcvdtime, &datalink, &offset);

  *len = head->caplen - offset;
  p += offset;
  if (offset && linknfo) {
    linknfo->datalinktype = datalink;
    linknfo->headerlen = offset;
    memcpy(linknfo->header, p - offset, MIN(sizeof(linknfo->header), offset));
  }
  *len = head->caplen - offset;
  return p;
}

bool get_pcap_result(NMAP_UltraScan* us, struct timeval* stime) {
  bool goodone = false;
  bool timedout = false;
  struct timeval rcvdtime;
  struct link_header linkhdr;
  unsigned int bytes;

  gettimeofday(&us->now, NULL);
  do {
    long to_usec = TIMEVAL_SUBTRACT(*stime, us->now);
    if (to_usec < 2000)
      to_usec = 2000;
    const struct ip* ip_tmp = (struct ip*)readip_pcap(us->handle, &bytes, to_usec, &rcvdtime, &linkhdr, true);
    gettimeofday(&us->now, NULL);
    if (ip_tmp == NULL && TIMEVAL_BEFORE(*stime, us->now)) {
      timedout = true;
      break;
    }
    if (ip_tmp == NULL)
      continue;
    if (TIMEVAL_SUBTRACT(us->now, *stime) > 200000)
      timedout = true;
    printf("ip proto = %d\n", ip_tmp->ip_p);
  }
  while (!goodone && !timedout);
  return goodone;
}


int64_t waitForResponses(NMAP_UltraScan* us) {
  struct timeval stime = {0};
  bool gotone;
  printf("In waitForResponses\n");
  do {
    printf("calling pcap_result\n");
    gotone = get_pcap_result(us, &stime);
  }
  while (gotone);
  return 0;
}

/**
 * @brief init_sniffer and apply filter to the sniffer.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t init_sniffer(NMAP_UltraScan* us) {
  const uint64_t nbrHosts = array_size(us->incompleteHosts);
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
    const char* str = inet_ntoa(*(struct in_addr*)array_cGet(us->incompleteHosts, targetno));
    strcat(dst_hosts, "src host ");
    strcat(dst_hosts, str);
  }
  us->handle = pcap_open_live(devs->name, 256, 1, 1000, errbuf);
  strcat(pcap_filter, "dst host ");
  strcat(pcap_filter, inet_ntoa(get_interface_ip(devs->name)));
  strcat(pcap_filter, " and (icmp or icmp6 or ((tcp) and (");
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
  printf("filter applied\n");
  free(fp.bf_insns);
  return 0;
}

/**
 * @brief ultra_scan
 * @param ips {Array<in_addr>} - Vector of targets to scan.
 * @param ports {Array<uint16_t>} - Vector of ports to scan.
 * @param scantype {NMAP_ScanType} - Type of scan to perform.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t ultra_scan(const Array* ips, const Array* ports, NMAP_ScanType scantype) {
  NMAP_UltraScan us = {0};

  us_default_init(&us);
  us.sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (us.sock < 0) {
    perror("socket/ultra_scan");
    return 1;
  }
  us.scanType = scantype;
  us_create_incHost(&us, ips, ports);
  us.nextIter = array_get(us.incompleteHosts, 0);
  // print imcomplete hosts
  (void)scantype;
  if (init_sniffer(&us)) {
    printf("init siffer failed\n");
    return 1;
  }
  while (true) {
    // do outstanding retransmit
    // do new probe
    if (doAnyNewProbe(&us))
      return 1;
    // check for responses
    if (waitForResponses(&us))
      return 1;
    break;
  }
  array_destroy(us.incompleteHosts);
  return 0;
}