#include <ft_nmap.h>

static int printPortElement(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)param;
  printf(" %u,", *(uint16_t*)value);
  return 0;
}

static int printIpElement(const Array* arr, size_t i, const void* value, void* param) {
  (void)arr, (void)i, (void)param;
  static char ipBuffer[INET_ADDRSTRLEN + 1] = {0};
  if (getnameinfo((void*)(struct sockaddr_in[]){{.sin_family = AF_INET, .sin_addr.s_addr = *(in_addr_t*)value}},
                  sizeof(struct sockaddr_in), ipBuffer, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST) == -1)
    return 1;
  printf("    \"%s\",\n", ipBuffer);
  return 0;
}

void NMAP_printOptions(const NMAP_Options* options) {
  puts("{\n"
       "  scan: [");

  if (options->scan & NMAP_SCAN_SYN)
    printf("    \"SYN\",\n");
  if (options->scan & NMAP_SCAN_ACK)
    printf("    \"ACK\",\n");
  if (options->scan & NMAP_SCAN_FIN)
    printf("    \"FIN\",\n");
  if (options->scan & NMAP_SCAN_NULL)
    printf("    \"NULL\",\n");
  if (options->scan & NMAP_SCAN_XMAS)
    printf("    \"XMAS\",\n");
  if (options->scan & NMAP_SCAN_UDP)
    printf("    \"UDP\",\n");

  printf("  ],\n"
         "  speedup: %u,\n"
         "  ips: [\n",
         options->speedup);

  array_cForEach(options->ips, printIpElement, NULL);

  puts("  ],\n"
       "  ports: [");

  array_cForEach(options->ports, printPortElement, NULL);
  puts("  ],\n}");
}

void NMAP_printWorkerOptions(const NMAP_WorkerOptions* options) {
  puts("{\n"
       "  scan: [");

  if (options->scan & NMAP_SCAN_SYN)
    printf("    \"SYN\",\n");
  if (options->scan & NMAP_SCAN_ACK)
    printf("    \"ACK\",\n");
  if (options->scan & NMAP_SCAN_FIN)
    printf("    \"FIN\",\n");
  if (options->scan & NMAP_SCAN_NULL)
    printf("    \"NULL\",\n");
  if (options->scan & NMAP_SCAN_XMAS)
    printf("    \"XMAS\",\n");
  if (options->scan & NMAP_SCAN_UDP)
    printf("    \"UDP\",\n");


  printf("  ],\n"
         "  ips: [\n");

  array_cForEach(options->ips, printIpElement, NULL);

  printf("  ],\n"
         "  ports: [\n");

  array_cForEach(options->ports, printPortElement, NULL);

  puts("  ],\n}");
}
