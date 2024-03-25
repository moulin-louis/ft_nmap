#define TOOLS_IMPL
#include <tools.h>

void memswap(void * a, void * b, size_t n) {
  static uint8_t swapBuffer[MEMSWAP_BUFFER_SIZE];

  for (size_t i = 0; i < n; i += MEMSWAP_BUFFER_SIZE) {
    size_t size = min((size_t)MEMSWAP_BUFFER_SIZE, n - i);

    memcpy(swapBuffer, a + i, size);
    memcpy(a + i, b + i, size);
    memcpy(b + i, swapBuffer, size);
  }
}
