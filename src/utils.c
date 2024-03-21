//
// Created by loumouli on 3/21/24.
//

#include "ft_nmap.h"

void ft_hexdump(const void* data, const uint64_t nbytes, const uint64_t row) {
  if (row == 0) {
    for (size_t i = 0; i < nbytes; i++) {
      printf("%02x ", ((uint8_t*)data)[i]);
    }
    printf("\n");
    return;
  }
  for (size_t i = 0; i < nbytes; i += row) {
    for (size_t j = i; j < i + row; j++) {
      if (j == nbytes) {
        break;
      }
      printf("%02x ", ((uint8_t*)data)[j]);
    }
    printf("\n");
  }
  printf("\n");
}

struct in_addr get_interface_ip(const char* ifname) {
  struct ifaddrs* ifaddr;
  const struct in_addr ipAddr = {0};

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  // Walk through linked list, maintaining head pointer so we can free list later
  for (const struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (strcmp(ifa->ifa_name, ifname) == 0 && ifa->ifa_addr->sa_family == AF_INET) { // Check it is IP4
      // is a valid IP4 Address
      freeifaddrs(ifaddr);
      return ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
    }
  }

  freeifaddrs(ifaddr);
  return ipAddr;
}