#include <array.h>

int array_set(Array * arr, ptrdiff_t pos, const void * value) {
  return array_fillWithin(arr, pos, pos + 1, value);
}
