#define ARRAY_IMPL
#include <array.h>

void array_setFactory(Array * arr, const ArrayFactory * factory) {
  assert(array != NULL && "array cannot be NULL");
  assert(factory != NULL && "factory cannot be NULL");

  arr->factory = *factory;
}
