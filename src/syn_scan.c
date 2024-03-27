//
// Created by loumouli on 3/20/24.
//

#include "ft_nmap.h"

bool timeout = false;

void tcp_syn_craft_payload(struct tcphdr* tcp_hdr, const uint16_t port) {
  tcp_hdr->source = htons(49152); // Source port, Likely unused port
  tcp_hdr->dest = htons(port); // Target port
  tcp_hdr->seq = 0;
  tcp_hdr->syn = 1;
  tcp_hdr->ack_seq = 0;
  tcp_hdr->window = htons(1024);
  tcp_hdr->doff = 5;
}

int32_t tcp_syn_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src) {
  struct sockaddr_in dest = {0};
  struct tcphdr tcp_hdr = {0};

  dest.sin_addr = ip_dest;
  dest.sin_port = htons(port->port);
  dest.sin_family = AF_INET;
  const int32_t sock = us->sock;
  tcp_syn_craft_payload(&tcp_hdr, port->port);
  tcp_hdr.check = tcp_checksum(&tcp_hdr, sizeof(tcp_hdr), ip_src, ip_dest);
  gettimeofday(&port->sendTime, NULL);
  const int64_t retval = send_packet(sock, (uint8_t*)&tcp_hdr, sizeof(tcp_hdr), 0, (struct sockaddr*)&dest);
  if (retval == -1) {
    perror("send_packet/retval");
    return 1;
  }
  printf("One probe sent\n");
  return 0;
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
