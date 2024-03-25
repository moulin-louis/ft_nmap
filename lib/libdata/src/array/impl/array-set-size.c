#define ARRAY_IMPL
#include <array.h>

void array_setSize(Array * array, size_t size) {
  assert(array != NULL && "array cannot be NULL");

  array->size = size;
}
