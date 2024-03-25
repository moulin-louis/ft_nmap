#define ARRAY_USE_IMPL
#include <array.h>

int array_defaultCtor(Array * arr, void * data, size_t size) {
  memset(data, 0, size * array_dataSize(arr));
  return 0;
}
