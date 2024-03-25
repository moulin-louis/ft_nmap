#include <ft_nmap.h>

uint32_t NMAP_getScanNumber(const char* name) {
  if (!strcmp(name, "SYN"))
    return NMAP_SCAN_SYN;
  if (!strcmp(name, "NULL"))
    return NMAP_SCAN_NULL;
  if (!strcmp(name, "FIN"))
    return NMAP_SCAN_FIN;
  if (!strcmp(name, "XMAS"))
    return NMAP_SCAN_XMAS;
  if (!strcmp(name, "ACK"))
    return NMAP_SCAN_ACK;
  if (!strcmp(name, "UDP"))
    return NMAP_SCAN_UDP;
  return NMAP_SCAN_NONE;
}
