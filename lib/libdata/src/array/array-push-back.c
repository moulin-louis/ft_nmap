#include <array.h>

int array_pushBack(Array * arr, const void * values, size_t count) {
  return array_push(arr, array_size(arr), values, count);
}
