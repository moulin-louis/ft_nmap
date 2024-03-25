#include <array.h>

int array_setupPos(const Array * arr, ptrdiff_t * pos) {
  if (*pos < 0)
    *pos += array_size(arr);
  if (*pos < 0 || (size_t)*pos > array_size(arr))
    goto OutOfRangeError;
  return ARRAY_SUCCESS;

OutOfRangeError:

  array_errno = ARR_ERANGE;
  return ARRAY_FAILURE;
}
