//
// Created by loumouli on 3/20/24.
//

#include "ft_nmap.h"

bool timeout = false;

pcap_t* handle = NULL;

void signal_handler(int sig) {
  (void)sig;
  pcap_breakloop(handle);
}

typedef struct {
  const NMAP_WorkerOptions* options;
  int32_t* sockets;
  Array* result;
} pcap_data;

int32_t init_sniffer(const Array* ips) {
  const uint64_t nbrHosts = array_size(ips);
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_if_t* devs;
  char dst_hosts[4096] = {0};
  char pcap_filter[4096] = {0};
  const bpf_u_int32 net = 0;
  struct bpf_program fp;

  if (pcap_findalldevs(&devs, errbuf) == -1) {
    fprintf(stderr, "pcap_findalldevs: %s\n", errbuf);
    return -1;
  }
  for (uint64_t targetno = 0; targetno < nbrHosts; ++targetno) {
    if (targetno == 0)
      strcat(dst_hosts, "");
    else
      strcat(dst_hosts, " or ");
    const char* str = inet_ntoa(*(struct in_addr*)array_cGet(ips, targetno));
    strcat(dst_hosts, "src host ");
    strcat(dst_hosts, str);
  }
  handle = pcap_open_live(devs->name, 256, 1, 1000, errbuf);
  strcat(pcap_filter, "dst host ");
  strcat(pcap_filter, inet_ntoa(get_interface_ip(devs->name)));
  strcat(pcap_filter, " and (icmp or icmp6 or ((tcp) and (");
  strcat(pcap_filter, dst_hosts);
  strcat(pcap_filter, ")))");
  printf("filter = [%s]\n", pcap_filter);
  if (pcap_compile(handle, &fp, pcap_filter, 0, net) == -1) {
    pcap_close(handle);
    fprintf(stdout, "Cant parse filter %s\n", pcap_geterr(handle));
    return 1;
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    pcap_close(handle);
    fprintf(stderr, "Couldnt apply filter %s\n", pcap_geterr(handle));
    return 1;
  }
  printf("filter applied\n");
  free(fp.bf_insns);
  return 0;
}

uint64_t tcp_syn_perform(const NMAP_WorkerOptions* options, int32_t VecSockets, Array* VecResult) {
  // handle = pcap_open_live(devs->name, 1024, 1, 1000, errbuf);
  (void)VecSockets;
  (void)VecResult;
  init_sniffer(options->ips);
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
    if (tcp_hdr->rst == 1)
      return CLOSE;
    if (tcp_hdr->ack == 1) {
      if (tcp_hdr->syn == 1)
        return OPEN;
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
