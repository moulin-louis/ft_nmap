//
// Created by loumouli on 3/20/24.
//

#include "ft_nmap.h"

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

uint64_t tcp_syn_perform(const NMAP_WorkerOptions* options, int32_t sockets[], NMAP_PortStatus* result) {
  for (uint16_t port = 0; port < options->nPorts; ++port) {
    uint8_t packet[sizeof(struct tcphdr)] = {0};
    struct tcphdr* tcp_hdr = (struct tcphdr*)&packet;
    struct sockaddr_in dest = {0};

    tcp_syn_craft_payload(tcp_hdr, port);
    dest.sin_family = AF_INET;
    dest.sin_port = tcp_hdr->dest;
    dest.sin_addr.s_addr = options->ip;
    tcp_hdr->check = tcp_checksum((uint16_t*)tcp_hdr, sizeof(struct tcphdr), get_interface_ip("eth0"), dest.sin_addr);
    int64_t retval = send_packet(sockets[port], packet, sizeof(packet), 0, (struct sockaddr*)&dest);
    if (retval == -1)
      return 1;
    uint8_t buff[4096] = {0};
    struct sockaddr_in sender = {0};
    retval = recv_packet(sockets[port], buff, 4096, 0, (struct sockaddr*)&sender);
    if (retval == -1)
      return 1;

    const struct iphdr* ip_hdr = (void*)buff;
    result[port] = tcp_syn_analysis(ip_hdr, buff + sizeof(struct iphdr));
    // need to send back, tcp packet with RESET bit set
    retval = tcp_syn_cleanup(sockets[port], packet, sizeof(packet), 0, (void*)&dest);
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
      if (tcp_hdr->syn == 1) {
        printf("Open port here\n");
        return OPEN;
      }
      if (tcp_hdr->rst == 1)
        return CLOSE;
    }
  }
  if (ip_hdr->protocol == IPPROTO_ICMP) {
    printf("Its a ICMP packet\n");
    const struct icmphdr* icmp_hdr = ip_payload;
    if (icmp_hdr->type == ICMP_DEST_UNREACH) {
      if (icmp_hdr->code == ICMP_HOST_UNREACH || icmp_hdr->code == ICMP_PROT_UNREACH ||
          icmp_hdr->code == ICMP_PORT_UNREACH || icmp_hdr->code == ICMP_NET_ANO || icmp_hdr->code == ICMP_HOST_ANO ||
          icmp_hdr->code == ICMP_PKT_FILTERED) {
        return FILTERED;
      }
    }
  }
  printf("Error here\n");
  return UNKOWN;
}

int64_t tcp_syn_cleanup(const int sck, uint8_t* packet, const uint64_t size_packet, const int32_t flag,
                        const struct sockaddr* dest) {
  struct tcphdr* tcp_hdr = (void*)packet;
  tcp_hdr->syn = 0;
  tcp_hdr->rst = 1;
  return send_packet(sck, packet, size_packet, flag, dest);
}
