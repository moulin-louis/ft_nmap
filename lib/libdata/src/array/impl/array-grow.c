#include <array.h>

int array_grow(Array * arr, size_t minCapacity) {
  size_t capacity = array_capacity(arr);

  if (capacity >= minCapacity)
    return 0;
  if (!capacity)
    capacity = 1;
  while (capacity < minCapacity)
    capacity <<= 1;

  return array_reserve(arr, capacity);
}
