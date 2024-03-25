#define ARRAY_IMPL
#include <array.h>

void * array_data(Array * arr) {
  assert(arr != NULL && "array cannot be NULL");

  return arr->data;
}

const void * array_cData(const Array * arr) {
  assert(arr != NULL && "array cannot be NULL");

  return arr->data;
}
