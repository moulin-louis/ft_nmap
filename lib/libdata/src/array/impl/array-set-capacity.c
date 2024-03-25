#define ARRAY_IMPL
#include <array.h>

void array_setCapacity(Array * array, size_t capacity) {
  assert(array != NULL && "array cannot be NULL");

  array->capacity = capacity;
}
