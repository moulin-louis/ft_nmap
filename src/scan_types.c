#include <ft_nmap.h>

static const char* const g_scanTypes[] = {
  [NMAP_SCAN_SYN] = "SYN",   [NMAP_SCAN_NULL] = "NULL", [NMAP_SCAN_FIN] = "FIN",
  [NMAP_SCAN_XMAS] = "XMAS", [NMAP_SCAN_ACK] = "ACK",   [NMAP_SCAN_UDP] = "UDP",
};

const char* NMAP_getScanName(NMAP_ScanType scan) {
  if ((size_t)scan >= COUNTOF(g_scanTypes))
    return NULL;
  return g_scanTypes[scan];
}

NMAP_ScanType NMAP_getScanNumber(const char* name) {
  for (size_t i = 0; i < COUNTOF(g_scanTypes); i++)
    if (g_scanTypes[i] && !strcmp(g_scanTypes[i], name))
      return i;
  return NMAP_SCAN_INVALID;
}
