#define ARRAY_IMPL
#include <array.h>

const ArrayFactory * array_getFactory(const Array * arr) {
  assert(arr != NULL && "array cannot be NULL");

  return &arr->factory;
}
