//
// Created by loumouli on 3/20/24.
//

#include "ft_nmap.h"

bool timeout = false;

typedef struct {
  const NMAP_WorkerOptions* options;
  int32_t* sockets;
  NMAP_PortStatus* result;
} pcap_data;

uint64_t tcp_syn_init(const uint16_t nPorts, int32_t sockets[]) {
  for (uint64_t i = 0; i < nPorts; ++i) {
    sockets[i] = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockets[i] == -1) {
      perror("socket");
      return 1;
    }
  }
  printf("All socket initialized\n");
  return 0;
}

void handle_pcap(uint8_t* user, const struct pcap_pkthdr* pkt, const uint8_t* bytes) {
  const pcap_data* data = (void*)user;
  if (pkt->len < sizeof(struct iphdr) + sizeof(struct tcphdr)) {
    return;
  }
  const struct iphdr* iphdr = (void*)bytes + sizeof(struct ether_header);
  const struct tcphdr* tcphdr = (void*)bytes + sizeof(struct ether_header) + sizeof(struct iphdr);
  ft_hexdump(tcphdr, sizeof(*tcphdr), 0);
  if (iphdr->protocol == IPPROTO_TCP) {
    printf("port src %d\n", ntohs(tcphdr->source));
    data->result[ntohs(tcphdr->source) - data->options->ports[0]] = tcp_syn_analysis(iphdr, tcphdr);
  }
}

int64_t analysis_network(const NMAP_WorkerOptions* options, int32_t sockets[], NMAP_PortStatus* result) {
  pcap_if_t* devs = NULL;
  pcap_t* handle = NULL;
  char errbuf[PCAP_ERRBUF_SIZE] = {0};
  char filter[4096] = {0};
  pcap_data data = {options, sockets, result};
  const bpf_u_int32 net = 0;
  struct bpf_program fp;
  if (pcap_findalldevs(&devs, errbuf)) {
    fprintf(stderr, "pcap_findalldevs: %s\n", errbuf);
    return 1;
  }
  handle = pcap_open_live(devs->name, 1024, 1, 1000, errbuf);
  if (handle == NULL) {
    fprintf(stderr, "pcap_openlive: %s\n", errbuf);
    return 1;
  }
  printf("handle opend !\n");
  char first_ip[256] = {0};
  memcpy(first_ip, inet_ntoa(get_interface_ip(devs->name)), sizeof(first_ip));

  pcap_freealldevs(devs);
  sprintf(filter, "dst host %s and src host %s", first_ip, inet_ntoa(*(struct in_addr*)&options->ip));
  if (pcap_compile(handle, &fp, filter, 0, net) == -1) {
    pcap_close(handle);
    fprintf(stdout, "Cant parse filter %s\n", pcap_geterr(handle));
    return 1;
  }
  printf("filter: [%s]\n", filter);
  if (pcap_setfilter(handle, &fp) == -1) {
    pcap_close(handle);
    fprintf(stderr, "Couldnt apply filter %s\n", pcap_geterr(handle));
    return 1;
  }
  pcap_freecode(&fp);
  fflush(NULL);
  pcap_loop(handle, options->nPorts, handle_pcap, (uint8_t*)&data);
  pcap_close(handle);
  return 0;
}

uint64_t tcp_syn_perform(const NMAP_WorkerOptions* options, int32_t sockets[], NMAP_PortStatus* result) {
  pcap_if_t* devs = NULL;
  pcap_findalldevs(&devs, NULL);
  for (uint64_t idx = 0; idx < options->nPorts; ++idx) {
    const uint16_t port = options->ports[idx];
    const int32_t sck = sockets[idx];
    uint8_t packet[sizeof(struct tcphdr)] = {0};
    struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
    struct sockaddr_in dest = {0};
    tcp_syn_craft_payload(tcp_hdr, port);
    dest.sin_family = AF_INET;
    dest.sin_port = tcp_hdr->dest;
    dest.sin_addr.s_addr = options->ip;
    tcp_hdr->check =
      tcp_checksum((uint16_t*)tcp_hdr, sizeof(struct tcphdr), get_interface_ip(devs->name), dest.sin_addr);
    const int64_t retval = send_packet(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest);
    if (retval == -1)
      return 1;
  }
  pcap_freealldevs(devs);
  printf("All probes have been sent !\n");
  analysis_network(options, sockets, result);
  for (uint64_t idx = 0; idx < options->nPorts; ++idx) {
    if (result[idx] != OPEN) {
      continue;
    }
    uint8_t packet[sizeof(struct tcphdr)] = {0};
    const uint16_t port = options->ports[idx];
    const int32_t sck = sockets[idx];
    struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
    struct sockaddr_in dest = {0};
    tcp_syn_craft_payload(tcp_hdr, port);
    dest.sin_family = AF_INET;
    dest.sin_port = tcp_hdr->dest;
    dest.sin_addr.s_addr = options->ip;
    tcp_hdr->syn = 0;
    tcp_hdr->rst = 1;
    tcp_hdr->check = tcp_checksum((uint16_t*)tcp_hdr, sizeof(struct tcphdr), get_interface_ip("eth0"), dest.sin_addr);
    const int64_t retval = send_packet(sck, packet, sizeof(packet), 0, (struct sockaddr*)&dest);
    if (retval == -1)
      return 1;
  }
  return 0;
}

void tcp_syn_craft_payload(struct tcphdr* tcp_hdr, const uint16_t port) {
  tcp_hdr->source = htons(49152); // Source port, Likely unused port
  tcp_hdr->dest = htons(port); // Target port
  tcp_hdr->seq = 0;
  tcp_hdr->syn = 1;
  tcp_hdr->ack_seq = 0;
  tcp_hdr->window = htons(1024);
  tcp_hdr->doff = 5;
}

NMAP_PortStatus tcp_syn_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  if (ip_hdr->protocol == IPPROTO_TCP) {
    const struct tcphdr* tcp_hdr = ip_payload;
    if (tcp_hdr->ack == 1) {
      if (tcp_hdr->syn == 1)
        return OPEN;
      if (tcp_hdr->rst == 1)
        return CLOSE;
    }
  }
  if (ip_hdr->protocol == IPPROTO_ICMP) {
    const struct icmphdr* icmp_hdr = ip_payload;
    if (icmp_hdr->type == ICMP_DEST_UNREACH) {
      if (icmp_hdr->code == ICMP_HOST_UNREACH || icmp_hdr->code == ICMP_PROT_UNREACH ||
          icmp_hdr->code == ICMP_PORT_UNREACH || icmp_hdr->code == ICMP_NET_ANO || icmp_hdr->code == ICMP_HOST_ANO ||
          icmp_hdr->code == ICMP_PKT_FILTERED) {
        return FILTERED;
      }
    }
  }
  return UNKOWN;
}
