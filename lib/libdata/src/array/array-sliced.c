#define ARRAY_USE_IMPL
#include <array.h>

Array * array_sliced(const Array * arr, ptrdiff_t from, ptrdiff_t to) {
  if (from < 0)
    from += array_size(arr);

  if (to < 0)
    to += array_size(arr);

  if (from < 0 || (size_t)from >= array_size(arr) || to < 0 ||
      (size_t)to > array_size(arr) || from > to)
    goto OutOfRangeError;

  if (from >= to)
    from = to;

  return array(
      array_dataSize(arr),
      to - from,
      to - from,
      array_cDataOffset(arr, from),
      array_getFactory(arr)
  );

OutOfRangeError:

  array_errno = ARR_ERANGE;
  return NULL;
}
