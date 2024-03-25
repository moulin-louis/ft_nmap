#define ARRAY_IMPL
#include <array.h>

void array_setData(Array * array, uint8_t * data) {
  assert(array != NULL && "array cannot be NULL");
  assert(data != NULL && "data cannot be NULL");

  array->data = data;
}
