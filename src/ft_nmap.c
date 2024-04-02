//
// Created by loumouli on 3/19/24.
//

#include <ft_nmap.h>

int main(int argc, char** argv) {
  if (getuid() != 0) {
    printf("Please run this program with root privileges\n");
    return NMAP_FAILURE;
  }
  NMAP_Options* options = NMAP_parseArgs(argc, argv);
  if (options == NULL)
    return NMAP_FAILURE;
  //  printf("GLOBAL OPTIONS\n");
  //   NMAP_printOptions(options);
  //  printf("--------------\n");
  return NMAP_spawnWorkers(options);
}
