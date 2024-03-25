#ifndef TOOLS_H
#define TOOLS_H

#ifdef TOOLS_IMPL
# include <assert.h>
# include <stddef.h>
# include <stdint.h>
# include <string.h>
#endif

#define MEMSWAP_BUFFER_SIZE 256

#define max(a, b)                                                              \
 ({                                                                            \
__typeof__(a) x = (a);                                                         \
__typeof__(b) y = (b);                                                         \
x > y ? x : y;                                                                 \
 })

#define min(a, b)                                                              \
 ({                                                                            \
__typeof__(a) x = (a);                                                         \
__typeof__(b) y = (b);                                                         \
x < y ? x : y;                                                                 \
 })

void memswap(void * a, void * b, size_t n);

#endif
