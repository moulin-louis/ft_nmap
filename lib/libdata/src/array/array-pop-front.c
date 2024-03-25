#include <array.h>

int array_popFront(Array * arr, size_t requestedCount, size_t * count) {
  return array_pop(arr, 0, requestedCount, count);
}
