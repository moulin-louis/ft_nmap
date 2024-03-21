//
// Created by loumouli on 3/19/24.
//

#include <ft_nmap.h>

static void NMAP_printOptions(const NMAP_Options* options) {
  static char ipBuffer[INET_ADDRSTRLEN];
  static const char* const scanTypes[] = {
    [NMAP_SCAN_SYN] = "SYN",   [NMAP_SCAN_NULL] = "NULL", [NMAP_SCAN_FIN] = "FIN",
    [NMAP_SCAN_XMAS] = "XMAS", [NMAP_SCAN_ACK] = "ACK",   [NMAP_SCAN_UDP] = "UDP",
  };

  getnameinfo((void*)(struct sockaddr_in[]){{.sin_family = AF_INET, .sin_addr.s_addr = options->ip}},
              sizeof(struct sockaddr_in), ipBuffer, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);

  printf("{\n"
         "  ip: \"%s\",\n"
         "  scan: \"%s\",\n"
         "  speedup: %u,\n"
         "  nPorts: %u,\n"
         "  ports: [",
         ipBuffer, scanTypes[options->scan], options->speedup, options->nPorts);
  for (uint16_t i = 0; i < options->nPorts; i++) {
    printf("%u", options->ports[i]);
    if (i != options->nPorts - 1)
      printf(", ");
  }
  puts("]\n}");
}

uint16_t port = 22;

struct in_addr get_interface_ip(const char* ifname) {
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

int main(int argc, char** argv) {
  (void)argc;
  uint8_t packet[sizeof(struct tcphdr)] = {0};

  struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
  struct sockaddr_in dest = {0};
  const int sck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sck == -1) {
    perror("socket SOCK RAW");
    return 1;
  }
  tcp_syn_craft_payload(tcp_hdr, port);
  dest.sin_family = AF_INET;
  dest.sin_port = tcp_hdr->dest;
  dest.sin_addr.s_addr = inet_addr(argv[1]);
  tcp_hdr->check = tcp_checksum((uint16_t*)tcp_hdr, sizeof(struct tcphdr), get_interface_ip("eth0"), dest.sin_addr);
  int64_t retval = send_packet(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest);
  if (retval == -1) {
    close(sck);
    return 1;
  }
  uint8_t buff[4096] = {0};
  struct sockaddr_in sender = {0};
  retval = recv_packet(sck, buff, sizeof(buff), 0, (struct sockaddr*)&sender);
  if (retval == -1) {
    close(sck);
    return 1;
  }

  const struct iphdr* ip_hdr = (void*)buff;
  if (ip_hdr->protocol == IPPROTO_TCP) {
  }
  const struct tcphdr* tcp_headr_recv = (void*)buff + sizeof(struct iphdr);
  switch (tcp_syn_analysis(NULL, tcp_headr_recv)) {
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
    return 1;
  }
  close(sck);
  return 0;
}
