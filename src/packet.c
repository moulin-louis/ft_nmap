//
// Created by loumouli on 3/21/24.
//

#include "ft_nmap.h"

uint64_t recv_packet(const int sck, uint8_t* packet, const uint64_t size_packet, const int32_t flag,
                     struct sockaddr* sender) {
  socklen_t len = sizeof(struct sockaddr);
  const int64_t retval = recvfrom(sck, packet, size_packet, flag, sender, &len);
  if (retval == -1) {
    // perror("recvfrom packet");
    return -1;
  }
  // struct tcphdr* tcp_hdr = (void*)packet + sizeof(struct iphdr);
  // struct iphdr* ip_hdr = (void*)packet;
  // printf("sck: %d, recv packet from %s:%u to %s:%u\n", sck, inet_ntoa(((struct sockaddr_in*)sender)->sin_addr), ntohs(tcp_hdr->source), inet_ntoa(*(struct in_addr*)&ip_hdr->daddr),ntohs(tcp_hdr->dest));
  // ft_hexdump(packet, retval, 0);
  // printf("\n");
  return 0;
}

uint64_t send_packet(const int sck, const uint8_t* packet, const uint64_t size_packet, const int32_t flag,
                     const struct sockaddr* dest) {
  const int64_t retval = sendto(sck, packet, size_packet, flag, dest, sizeof(struct sockaddr));
  if (retval == -1) {
    return -1;
  }
  // const struct tcphdr* tcp_hdr = (void *)packet;
  // printf("sck: %d, send %ld bytes to %s:%u\n", sck, retval, inet_ntoa(((struct sockaddr_in*)dest)->sin_addr),ntohs(tcp_hdr->dest));
  // ft_hexdump(packet, retval, 0);
  // printf("\n");
  return 0;
}
