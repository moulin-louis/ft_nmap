#define ARRAY_USE_IMPL
#include <array.h>

void * array_get(Array * arr, ptrdiff_t pos) {
  assert(arr != NULL && "array cannot be NULL");

  if (pos < 0)
    pos += array_size(arr);
  if (pos < 0 || (size_t)pos >= array_size(arr))
    goto OutOfRangeError;
  return array_dataOffset(arr, pos);

OutOfRangeError:
  array_errno = ARR_ERANGE;
  return NULL;
}

const void * array_cGet(const Array * arr, ptrdiff_t pos) {
  return array_get((void *)arr, pos);
}
