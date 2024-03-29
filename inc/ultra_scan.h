//
// Created by loumouli on 3/27/24.
//

#ifndef ULTRA_SCAN_H
#define ULTRA_SCAN_H

#include "ft_nmap.h"

/* Timeval subtraction in microseconds */
#define TIMEVAL_SUBTRACT(a, b) (((a).tv_sec - (b).tv_sec) * 1000000 + (a).tv_usec - (b).tv_usec)
#define TIMEVAL_BEFORE(a, b) (((a).tv_sec < (b).tv_sec) || ((a).tv_sec == (b).tv_sec && (a).tv_usec < (b).tv_usec))
#define TIMEVAL_TO_MICROSC(a) ((a).tv_sec * 1000000 + (a).tv_usec)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief Structure to store all the information needed for ultra_scan engine.
 * @param {pcap_t*} handle - Pcap handle.
 * @param {struct in_addr} inter_ip - IP address of the interface used for pcap handle.
 * @param {int32_t} sock - raw socket file descriptor.
 * @param {Array<t_host>} hosts - Vector of hosts to scan.
 * @param {uint64_t} idxNextHost - Index of the next host to scan.
 * @param {NMAPP_ScanType} scanType - Type of scan to perform
 * @param {double} srtt - Smoothed Round-Trip Time in microseconds.
 * @param {double} rttvar - Round-Trip Time Variance in microseconds.
 * @param {double} timeout - Timeout for a probe in microseconds.
 * @param {double} maxTimeout - Maximum timeout for a probe in microseconds.
 * @param {double} minTimeout - Minimum timeout for a probe in microseconds.
 * @param {uint64_t} maxRetries - Maximum number of retries for a probe.
 * @param {struct timeval} now - Current time.
 */
typedef struct {
  pcap_t* handle;
  struct in_addr inter_ip;
  int32_t sock;
  Array* hosts;
  uint64_t idxNextHosts;
  NMAP_ScanType scanType;
  long double srtt;
  long double rttvar;
  long double timeout;
  double maxTimeout;
  double minTimeout;
  uint64_t maxRetries;
  struct timeval now;
  uint64_t packet_recv;
} NMAP_UltraScan;

const char* inet_ntop_ez(const struct sockaddr_storage* ss, size_t sslen);

#endif // ULTRA_SCAN_H
