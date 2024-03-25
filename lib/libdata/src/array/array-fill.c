#define ARRAY_USE_IMPL
#include <array.h>

int array_fill(Array * arr, const void * value) {
  return array_fillWithin(arr, 0, array_size(arr), value);
}

int array_fillWithin(
    Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    goto AnyError;
  if (from == to)
    return ARRAY_SUCCESS;

  ptrdiff_t i = from;
  array_getFactory(arr)->destructor(arr, array_get(arr, from), to - from);
  if (array_getFactory(arr)->copyConstructor(
          arr, array_dataOffset(arr, from), value, 1
      ))
    goto CopyConstructorError;

  ++i;
  while (i < to) {
    size_t n = min(i - from, to - i);
    if (array_getFactory(arr)->copyConstructor(
            arr, array_dataOffset(arr, i), array_cDataOffset(arr, from), n
        ))
      goto CopyConstructorError;
    i += n;
  }
  return ARRAY_SUCCESS;

CopyConstructorError:

  array_getFactory(arr)->destructor(arr, array_dataOffset(arr, from), i - from);
  memcpy(
      array_dataOffset(arr, i), array_cDataOffset(arr, to), array_size(arr) - to
  );
  array_setSize(arr, i + array_size(arr) - to);
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}
