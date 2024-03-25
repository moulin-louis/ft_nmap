#include <array.h>

void * array_front(Array * arr) {
  if (array_empty(arr))
    goto OutOfRangeError;

  return array_data(arr);

OutOfRangeError:

  array_errno = ARR_ERANGE;
  return NULL;
}

const void * array_cFront(const Array * arr) {
  return array_front((void *)arr);
}
