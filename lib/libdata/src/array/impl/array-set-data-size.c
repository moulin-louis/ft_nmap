#define ARRAY_IMPL
#include <array.h>

void array_setDataSize(Array * array, size_t dataSize) {
  assert(array != NULL && "array cannot be NULL");
  assert(dataSize != 0 && "dataSize cannot be 0");

  array->dataSize = dataSize;
}
