//
// Created by loumouli on 3/20/24.
//

#include "ft_nmap.h"

void tcp_syn_craft_payload(struct tcphdr* tcp_hdr, const uint16_t port) {
  tcp_hdr->source = htons(49152); // Source port is this. Likely unused port
  tcp_hdr->dest = htons(port); // Targeting port  (SSH)
  tcp_hdr->seq = 0;
  tcp_hdr->syn = 1;
  tcp_hdr->ack_seq = 0;
  tcp_hdr->window = htons(2);
  tcp_hdr->doff = 5;
}

NMAP_PortStatus tcp_syn_analysis(struct tcphdr* tcp_hdr) {

  if (tcp_hdr->ack == 1 && tcp_hdr->syn == 1) {
    //Port is OPEN
    return OPEN;
  }
  if (tcp_hdr->ack == 1 && tcp_hdr->rst == 1) {
    return CLOSE;
  }
  return FILTERED;
}
