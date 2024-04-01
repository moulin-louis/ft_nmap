//
// Created by loumouli on 3/29/24.
//

#include "ft_nmap.h"

static NMAP_PortStatus icmp_analysis(const struct icmphdr* icmp_hdr) {
  if (icmp_hdr->type == ICMP_DEST_UNREACH) {
    if (icmp_hdr->code == ICMP_HOST_UNREACH || icmp_hdr->code == ICMP_PROT_UNREACH ||
        icmp_hdr->code == ICMP_PORT_UNREACH || icmp_hdr->code == ICMP_NET_ANO || icmp_hdr->code == ICMP_HOST_ANO ||
        icmp_hdr->code == ICMP_PKT_FILTERED) {
      return NMAP_FILTERED;
    }
  }
  return NMAP_UNKOWN;
}

NMAP_PortStatus tcp_syn_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  if (ip_hdr->protocol == IPPROTO_TCP) {
    const struct tcphdr* tcp_hdr = ip_payload;
    if (tcp_hdr->rst == 1)
      return NMAP_CLOSE;
    if (tcp_hdr->ack == 1) {
      if (tcp_hdr->syn == 1)
        return NMAP_OPEN;
    }
  }
  else if (ip_hdr->protocol == IPPROTO_ICMP)
    return icmp_analysis(ip_payload);
  return NMAP_UNKOWN;
}

NMAP_PortStatus tcp_ack_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  if (ip_hdr->protocol == IPPROTO_TCP) {
    if (ip_hdr->protocol == IPPROTO_TCP) {
      const struct tcphdr* tcp_hdr = ip_payload;
      if (tcp_hdr->rst == 1)
        return NMAP_CLOSE;
    }
  }
  else if (ip_hdr->protocol == IPPROTO_ICMP)
    return icmp_analysis(ip_payload);
  return NMAP_UNKOWN;
}

NMAP_PortStatus tcp_fnx_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  if (ip_hdr->protocol == IPPROTO_TCP) {
    const struct tcphdr* tcp_hdr = ip_payload;
    if (tcp_hdr->rst == 1)
      return NMAP_CLOSE;
  }
  else if (ip_hdr->protocol == IPPROTO_ICMP)
    return icmp_analysis(ip_payload);
  return NMAP_UNKOWN;
}

NMAP_PortStatus tcp_fin_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  return tcp_fnx_analysis(ip_hdr, ip_payload);
}

NMAP_PortStatus tcp_xmas_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  return tcp_fnx_analysis(ip_hdr, ip_payload);
}

NMAP_PortStatus tcp_null_analysis(const struct iphdr* ip_hdr, const void* ip_payload) {
  return tcp_fnx_analysis(ip_hdr, ip_payload);
}
