#define ARRAY_IMPL
#include <array.h>

size_t array_capacity(const Array * arr) {
  assert(arr != NULL && "array cannot be NULL");

  return arr->capacity;
}
