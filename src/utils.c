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
  struct in_addr ipAddr = {0};

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
      ipAddr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
      freeifaddrs(ifaddr);
      return ipAddr;
    }
  }

  freeifaddrs(ifaddr);
  return ipAddr;
}

char* port_status_to_string(const NMAP_PortStatus status) {
  switch (status) {
  case OPEN:
    return "OPEN";
  case CLOSE:
    return "CLOSE";
  case FILTERED:
    return "FILTERED";
  case UNFILTERED:
    return "UNFILTERED";
  case UNKOWN:
    return "UNKNOW";
  }
  return "REALY REALY WEIRD STUFF IS HAPENING RIGHT NOW IN PORT STATUS TO STRING";
}

const char *inet_ntop_ez(const struct sockaddr_storage *ss, size_t sslen) {

  const struct sockaddr_in *sin = (struct sockaddr_in *) ss;
  static char str[INET6_ADDRSTRLEN];
  const struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) ss;

  str[0] = '\0';

  if (sin->sin_family == AF_INET) {
    if (sslen < sizeof(struct sockaddr_in))
      return NULL;
    return inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str));
  }
  else if(sin->sin_family == AF_INET6) {
    if (sslen < sizeof(struct sockaddr_in6))
      return NULL;
    return inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str));
  }
  //Some laptops report the ip and address family of disabled wifi cards as null
  //so yes, we will hit this sometimes.
  return NULL;
}
