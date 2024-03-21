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