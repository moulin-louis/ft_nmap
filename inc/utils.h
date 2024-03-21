#ifndef UTILS_H
#define UTILS_H

#define COUNTOF(array) (sizeof(array) / sizeof(*array))

struct in_addr get_interface_ip(const char* ifname);

void ft_hexdump(const void* data, uint64_t nbytes, uint64_t row);

#endif
