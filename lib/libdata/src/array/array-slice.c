#define ARRAY_USE_IMPL
#include <array.h>

int array_slice(Array * arr, ptrdiff_t from, ptrdiff_t to) {
  if (from < 0)
    from += array_size(arr);

  if (to < 0)
    to += array_size(arr);

  if (from < 0 || (size_t)from >= array_size(arr) || to < 0 ||
      (size_t)to > array_size(arr) || from > to)
    goto OutOfRangeError;

  if (from >= to)
    from = to;

  array_getFactory(arr)->destructor(arr, array_data(arr), from);
  array_getFactory(arr)->destructor(
      arr, array_data(arr) + to * array_dataSize(arr), array_size(arr) - to
  );
  array_setSize(arr, to - from);
  memcpy(
      array_data(arr),
      array_cDataOffset(arr, from),
      array_size(arr) * array_dataSize(arr)
  );

  return 0;

OutOfRangeError:

  array_errno = ARR_ERANGE;
  return 1;
}
