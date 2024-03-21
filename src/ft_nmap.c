//
// Created by loumouli on 3/19/24.
//

#include <ft_nmap.h>

static void NMAP_printOptions(const NMAP_Options* options) {
  static char ipBuffer[INET_ADDRSTRLEN];

  getnameinfo((void*)(struct sockaddr_in[]){{.sin_family = AF_INET, .sin_addr.s_addr = options->ip}},
              sizeof(struct sockaddr_in), ipBuffer, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);

  printf("{\n"
         "  ip: \"%s\",\n"
         "  scan: \"%s\",\n"
         "  speedup: %u,\n"
         "  nPorts: %u,\n"
         "  ports: [",
         ipBuffer, NMAP_getScanName(options->scan), options->speedup, options->nPorts);
  for (uint16_t i = 0; i < options->nPorts; i++) {
    printf("%u", options->ports[i]);
    if (i != options->nPorts - 1)
      printf(", ");
  }
  puts("]\n}");
}

int main(int argc, char** argv) {
  (void)argc;
  uint8_t packet[sizeof(struct tcphdr)] = {0};

  NMAP_Options options = NMAP_parseArgs(argc, argv);

  if (NMAP_SpawnWorkers(&options) == NMAP_FAILURE)
    return 1;
  return 0;
}
