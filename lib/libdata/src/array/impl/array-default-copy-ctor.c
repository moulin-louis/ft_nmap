#define ARRAY_USE_IMPL
#include <array.h>

int array_defaultCopyCtor(Array * arr, void * dst, const void * src, size_t n) {
  memcpy(dst, src, n * array_dataSize(arr));
  return 0;
}
