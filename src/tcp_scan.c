//
// Created by loumouli on 3/21/24.
//

#include "ft_nmap.h"

typedef struct {
  uint32_t src_addr;
  uint32_t dest_addr;
  uint8_t pholder;
  uint8_t protocol;
  uint16_t tcp_len;
} TCP_PseudoHeader;

void tcp_craft_payload(struct tcphdr* tcp_hdr, const uint16_t port) {
  tcp_hdr->source = htons(49152); // Source port, Likely unused port
  tcp_hdr->dest = htons(port); // Target port
  tcp_hdr->seq = 0;
  tcp_hdr->syn = 1;
  tcp_hdr->ack_seq = 0;
  tcp_hdr->window = htons(1024);
  tcp_hdr->doff = 5;
}


int32_t tcp_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src,
                       uint16_t tcp_flag) {
  struct sockaddr_in dest = {0};
  struct tcphdr tcp_hdr = {0};

  dest.sin_addr = ip_dest;
  dest.sin_port = htons(port->port);
  dest.sin_family = AF_INET;
  const int32_t sock = us->sock;
  tcp_craft_payload(&tcp_hdr, port->port);
  tcp_hdr.th_flags = tcp_flag;
  tcp_hdr.check = tcp_checksum(&tcp_hdr, sizeof(tcp_hdr), ip_src, ip_dest);
  gettimeofday(&port->sendTime, NULL);
  const int64_t retval = send_packet(sock, (uint8_t*)&tcp_hdr, sizeof(tcp_hdr), 0, (struct sockaddr*)&dest);
  if (retval == -1) {
    perror("send_packet/retval");
    return 1;
  }
  return 0;
}

int32_t tcp_syn_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src) {
  return tcp_send_probe(us, port, ip_dest, ip_src, TH_SYN);
}

int32_t tcp_ack_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src) {
  return tcp_send_probe(us, port, ip_dest, ip_src, TH_ACK);
}

int32_t tcp_fin_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src) {
  return tcp_send_probe(us, port, ip_dest, ip_src, TH_FIN);
}

int32_t tcp_xmas_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src) {
  return tcp_send_probe(us, port, ip_dest, ip_src, TH_FIN | TH_PUSH | TH_URG);
}

int32_t tcp_null_send_probe(const NMAP_UltraScan* us, t_port* port, struct in_addr ip_dest, struct in_addr ip_src) {
  return tcp_send_probe(us, port, ip_dest, ip_src, 0);
}

uint16_t checksum(uint16_t* buffer, int size) {
  uint64_t cksum = 0;
  while (size > 1) {
    cksum += *buffer++;
    size -= sizeof(uint16_t);
  }
  if (size)
    cksum += *(uint8_t*)buffer;

  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += cksum >> 16;
  return ~cksum;
}

uint16_t tcp_checksum(const void* vdata, const size_t length, const struct in_addr src_addr,
                      const struct in_addr dest_addr) {
  // Create the pseudo header
  TCP_PseudoHeader psh = {0};
  uint8_t pseudogram[1024] = {0};
  psh.src_addr = src_addr.s_addr;
  psh.dest_addr = dest_addr.s_addr;
  psh.pholder = 0;
  psh.protocol = IPPROTO_TCP;
  psh.tcp_len = htons(length);

  // Calculate the size for the pseudo header + TCP header
  memcpy(pseudogram, (char*)&psh, sizeof(TCP_PseudoHeader));
  memcpy(pseudogram + sizeof(TCP_PseudoHeader), vdata, length);
  const uint16_t chcksm = checksum((uint16_t*)pseudogram, sizeof(TCP_PseudoHeader) + length);
  return chcksm;
}
