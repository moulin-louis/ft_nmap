//
// Created by loumouli on 3/21/24.
//

#include "ft_nmap.h"

uint64_t send_packet(const int sck, const uint8_t* packet, const uint64_t size_packet, const int32_t flag,
                     const struct sockaddr* dest) {
  const int64_t retval = sendto(sck, packet, size_packet, flag, dest, sizeof(struct sockaddr));
  if (retval == -1)
    return -1;
  return 0;
}
