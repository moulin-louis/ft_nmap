#define ARRAY_IMPL
#include <array.h>

int array_emplaceFront(Array * arr, size_t count) {
  return array_emplace(arr, 0, count);
}
