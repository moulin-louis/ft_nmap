#include <array.h>

int array_popBack(Array * arr, size_t requestedCount, size_t * count) {
  return array_pop(arr, -requestedCount, requestedCount, count);
}
