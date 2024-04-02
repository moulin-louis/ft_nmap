//
// Created by loumouli on 3/27/24.
//

#ifndef ULTRA_SCAN_H
#define ULTRA_SCAN_H

/* Timeval subtraction in microseconds */
#define TIMEVAL_SUBTRACT(a, b) (((a).tv_sec - (b).tv_sec) * 1000000 + (a).tv_usec - (b).tv_usec)
#define TIMEVAL_TO_MICROSC(a) ((a).tv_sec * 1000000 + (a).tv_usec)

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
  uint64_t packet_sent;
  uint64_t packet_retransmit;
  uint64_t port_timeout;
} NMAP_UltraScan;

/**
 * @brief update the SRTT of the NMAP_UltraScan structure based on the probe received.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan struc
 * @param port {const t_port*} - Probe recieved to update the SRTT
 */
void us_updateSRTT(NMAP_UltraScan* us, const t_port* port);


/**
 * @brief update the RTTVAR of the NMAP_UltraScan structure based on the probe received.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan struc
 * @param port {const t_port*} - Probe recieved to update the RTTVAR
 */
void us_updateRTTVAR(NMAP_UltraScan* us, const t_port* port);

/**
 * @brief update the timeout of the NMAP_UltraScan structure based on the SRTT and RTTVAR.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param port {const t_port*} - Probe received to update the timeout.
 */
void us_updateTimeout(NMAP_UltraScan* us, const t_port* port);

/**
 * @brief init NMAP_UltraScan structure to default value.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure to initialize.
 */
void us_default_init(NMAP_UltraScan* us);

/**
 * @brief create host vectors based on input ips and ports.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure to initialize.
 * @param ips  {Array<struct addr_in>} - Vector of IP addresses to filter.
 * @param ports {Array<uint16_t>} - Vector of ports to scan.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t us_createHost(NMAP_UltraScan* us, const Array* ips, const Array* ports);

/**
 * @brief init_sniffer and apply filter to the sniffer.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t init_sniffer(NMAP_UltraScan* us);

/**
 * @brief return the next host to scan and increment the nextIter.
 * @param us {NMAP_UltraScan*} UltraScan structure.
 * @return {t_host*} - Next host to scan.
 */
t_host* us_nextHost(NMAP_UltraScan* us);

/**
 * @brief send a probe to the next port of a given host
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param host {t_host*} - Host to send the probe to.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t sendNextScanProbe(NMAP_UltraScan* us, t_host* host);

/**
 * @brief send probe to any needed target in targets.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t doAnyNewProbe(NMAP_UltraScan* us);

/**
 * @brief wait for the fd of the pcap handle to be ready.
 * @param p {pcap_t*} - pcap handler to use
 * @param to_usec {long} - timeout to wait for in microseconds.
 * @return {int64_t} - 0 if timeout, -1 on error, > 0 if fd is ready.
 */
int64_t pcap_poll(pcap_t* p, int64_t to_usec);

/**
 * @brief read a packet from the pcap handle with a timeout.
 * @param handle {pcap_t*} - pcap handle
 * @param to_usec {long} - timeout in microseconds
 * @param packet {const uint8_t**} - pointer to a uint8_t pointer
 * @param head {struct pcap_pkthdr**} - pointer to a pcap header
 * @param rcvdtime  {struct timeval*} - time when the packet was received
 * @return {int64_t} - 0 if success, 1 otherwise.
 */
int64_t read_reply_pcap(pcap_t* handle, int64_t to_usec, const uint8_t** packet, struct pcap_pkthdr** head,
                        struct timeval* rcvdtime);

/**
 * @brief grap a packet from the pcap handle and process it.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 * @param stime {struct timeval*} - start time.
 * @return {bool} - true if there is a result, false otherwise.
 */
bool get_pcap_result(NMAP_UltraScan* us, const struct timeval* stime);

/**
 * @brief recv and process packet until there is no more packet to process or timeout.
 * @param us {NMAP_UltraScan*} - NMAP_UltraScan structure.
 */
void waitForResponses(NMAP_UltraScan* us);

/**
 * @brief - Handle timeout for sent probe and check the number of retries
 * @param us {NMAP_Ultrascan*} - UltraScan structure
 */
void doAnyOustandingRetransmit(NMAP_UltraScan* us);
#endif // ULTRA_SCAN_H
