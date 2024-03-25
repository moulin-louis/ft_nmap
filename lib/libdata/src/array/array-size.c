#define ARRAY_IMPL
#include <array.h>

size_t array_size(const Array * arr) {
  assert(arr != NULL && "array cannot be NULL");

  return arr->size;
}
