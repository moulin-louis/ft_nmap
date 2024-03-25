#define ARRAY_USE_IMPL
#include <array.h>

int array_rotate(Array * arr, ptrdiff_t n) {
  return array_rotateWithin(arr, 0, array_size(arr), n);
}

int array_rotateWithin(Array * arr, ptrdiff_t from, ptrdiff_t to, ptrdiff_t n) {
  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  const size_t size = to - from;
  const size_t absN = llabs(n) % size;

  if (size < 2 || !absN)
    return 0;

  n = n < 0 ? -absN : absN;
  if (n < 0)
    n += size;

  char * tmp = malloc((size_t)(array_dataSize(arr) * n));

  if (!tmp)
    goto NoMemoryError;

  memcpy(tmp, array_cDataOffset(arr, from), array_dataSize(arr) * n);
  memcpy(
      array_dataOffset(arr, from),
      array_cDataOffset(arr, from + n),
      array_dataSize(arr) * (size - n)
  );
  memcpy(array_dataOffset(arr, to - n), tmp, array_dataSize(arr) * n);
  free(tmp);

  return ARRAY_SUCCESS;

NoMemoryError:

  array_errno = ARR_ENOMEM;
  return ARRAY_FAILURE;
}
