//
// Created by loumouli on 3/19/24.
//

#include "ft_nmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>


int main(int ac, char **av) {
  (void)ac;
  (void)av;
  int sck = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sck == -1) {
    perror("socket SOCK RAW");
    return 1;
  }
  printf("sck = %d\n", sck);
  return 0;
}
