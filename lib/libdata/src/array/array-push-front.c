#include <array.h>

int array_pushFront(Array * arr, const void * values, size_t count) {
  return array_push(arr, 0, values, count);
}
