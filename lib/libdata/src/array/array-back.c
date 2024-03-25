#include <array.h>

void * array_back(Array * arr) {
  if (array_empty(arr))
    goto OutOfRangeError;

  return array_get(arr, -1);

OutOfRangeError:

  array_errno = ARR_ERANGE;
  return NULL;
}

const void * array_cBack(const Array * arr) { return array_back((void *)arr); }
