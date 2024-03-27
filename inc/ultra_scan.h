//
// Created by loumouli on 3/27/24.
//

#ifndef ULTRA_SCAN_H
#define ULTRA_SCAN_H

#include "ft_nmap.h"

/**
 * @brief Structure to store all the information needed for ultra_scan engine.
 * @param {Array<struct s_hosts>} incompleteHosts - Vector of hosts to scan.
 * @param {UsHosts*} nextIter - Next host to scan.
 * @param {double} srtt - Smoothed Round-Trip Time.
 * @param {double} rttvar - Round-Trip Time Variance.
 * @param {double} timeout - Timeout for a probe.
 * @param {double} maxTimeout - Maximum timeout for a probe.
 * @param {double} minTimeout - Minimum timeout for a probe.
 * @param {uint64_t} maxRetries - Maximum number of retries for a probe.
 */
typedef struct {
  pcap_t* handle;
  struct in_addr inter_ip;
  int32_t sock;
  Array* incompleteHosts;
  t_host* nextIter;
  NMAP_ScanType scanType;
  double srtt;
  double rttvar;
  double timeout;
  double maxTimeout;
  double minTimeout;
  uint64_t maxRetries;
  struct timeval now;
} NMAP_UltraScan;

struct abstract_ip_hdr {
  uint8_t version; /* 4 or 6. */
  struct sockaddr_storage src;
  struct sockaddr_storage dst;
  uint8_t proto; /* IPv4 proto or IPv6 next header. */
  uint8_t ttl;   /* IPv4 TTL or IPv6 hop limit. */
  uint32_t ipid; /* IPv4 IP ID or IPv6 flow label. */
};
void us_default_init(NMAP_UltraScan* us);

const char* inet_ntop_ez(const struct sockaddr_storage* ss, size_t sslen);

#endif // ULTRA_SCAN_H
