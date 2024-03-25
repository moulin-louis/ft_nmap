//
// Created by loumouli on 3/19/24.
//

#include <ft_nmap.h>

int main(int argc, char** argv) {
  (void)argc, (void)argv;
  NMAP_Options* options = NMAP_parseArgs(argc, argv);

  if (!options)
    return NMAP_FAILURE;
  printf("GLOBAL OPTIONS\n");
  NMAP_printOptions(options);
  printf("--------------\n");
  if (NMAP_spawnWorkers(options) == NMAP_FAILURE)
    return NMAP_FAILURE;
  return NMAP_SUCCESS;
}
