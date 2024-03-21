#include <ft_nmap.h>

static struct in_addr get_interface_ip(const char* ifname) {
  struct ifaddrs* ifaddr;
  const struct in_addr ipAddr = {0};

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  // Walk through linked list, maintaining head pointer so we can free list later
  for (const struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (strcmp(ifa->ifa_name, ifname) == 0 && ifa->ifa_addr->sa_family == AF_INET) { // Check it is IP4
      // is a valid IP4 Address
      freeifaddrs(ifaddr);
      return ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
    }
  }

  freeifaddrs(ifaddr);
  return ipAddr;
}

static void* NMAP_workerMain(void* arg) {
  const NMAP_WorkerOptions* const options = arg;
  int sockets[options->nPorts];

  (void)sockets;

  const uint16_t port = 22;

  uint8_t packet[sizeof(struct tcphdr)] = {0};

  struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
  struct sockaddr_in dest = {0};
  const int sck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sck == -1) {
    perror("socket SOCK RAW");
    return NULL;
  }
  tcp_syn_craft_payload(tcp_hdr, port);
  dest.sin_family = AF_INET;
  dest.sin_port = tcp_hdr->dest;
  dest.sin_addr.s_addr = options->ip;
  tcp_hdr->check = tcp_checksum((uint16_t*)tcp_hdr, sizeof(struct tcphdr), get_interface_ip("eth0"), dest.sin_addr);
  int64_t retval = send_packet(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest);
  if (retval == -1) {
    close(sck);
    return NULL;
  }
  uint8_t buff[4096] = {0};
  struct sockaddr_in sender = {0};
  retval = recv_packet(sck, buff, sizeof(buff), 0, (struct sockaddr*)&sender);
  if (retval == -1) {
    close(sck);
    return NULL;
  }

  const struct iphdr* ip_hdr = (void*)buff;
  if (ip_hdr->protocol == IPPROTO_TCP) {
  }
  switch (tcp_syn_analysis(NULL, buff + sizeof(struct iphdr))) {
  case OPEN:
    printf("Port %d is open\n", port);
    break;
  case CLOSE:
    printf("Port %d is close\n", port);
    break;
  case FILTERED:
    printf("Port %d is filtered\n", port);
    break;
  default: {
  }
  }
  // need to send back, tcp packet with RESET bit set
  retval = tcp_syn_cleanup(sck, packet, sizeof(packet), 0, (void*)&dest);
  if (retval == -1) {
    close(sck);
    return NULL;
  }
  close(sck);

  // need to return something allocated with malloc, or NULL, in case of failure
  void* result = malloc(0);
  if (!result) {
    perror("malloc");
    return NULL;
  }

  return result;
}

int NMAP_spawnWorkers(const NMAP_Options* options) {
  pthread_t workers[options->speedup];
  NMAP_WorkerOptions workerOptions[options->speedup];
  void* workerResults[options->speedup];
  uint16_t maxPortsPerWorker = options->nPorts / options->speedup;
  uint8_t nThreads = 0;
  uint16_t portsLeft = options->nPorts;

  if (!maxPortsPerWorker)
    maxPortsPerWorker = 1;
  for (uint8_t i = 0; portsLeft && i < options->speedup; ++i) {
    ++nThreads;
    workerOptions[i].ip = options->ip;
    workerOptions[i].scan = options->scan;
    workerOptions[i].nPorts = maxPortsPerWorker;
    if (portsLeft < maxPortsPerWorker)
      workerOptions[i].nPorts = portsLeft;
    for (uint16_t j = 0; j < workerOptions[i].nPorts; ++j)
      workerOptions[i].ports[j] = options->ports[workerOptions[i].nPorts - portsLeft--];
  }
  for (uint8_t i = 0; i < nThreads; ++i)
    if (pthread_create(&workers[i], NULL, NMAP_workerMain, (void*)&workerOptions[i])) {
      for (size_t j = 0; j < i; ++j)
        pthread_cancel(workers[j]);
      for (size_t j = 0; j < i; ++j)
        pthread_join(workers[j], NULL);
      perror("ft_nmap: failed to spawn a thread");
      return NMAP_FAILURE;
    }
  for (size_t i = 0; i < nThreads; ++i) {
    if (pthread_join(workers[i], &workerResults[i]))
      perror("ft_nmap: failed to join a thread");
    if (!workerResults[i])
      fputs("ft_nmap: an error occured in a worker thread\n", stderr);
  }
  for (size_t i = 0; i < nThreads; ++i) {
    // do something with results
    free(workerResults[i]);
  }
  return NMAP_SUCCESS;
}
