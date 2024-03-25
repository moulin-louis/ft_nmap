#define ARRAY_USE_IMPL
#include <array.h>

int array_copy(Array * arr, const void * values, size_t count) {
  return array_copyWithin(arr, 0, array_size(arr), values, count);
}

int array_copyWithin(
    Array * arr, ptrdiff_t from, ptrdiff_t to, const void * values, size_t count
) {
  assert(values != NULL && "values cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    goto AnyError;

  const size_t n = min((size_t)(to - from), count);

  array_getFactory(arr)->destructor(arr, array_get(arr, from), n);
  if (array_getFactory(arr)->copyConstructor(
          arr, array_dataOffset(arr, from), values, n
      ))
    goto CopyConstructorError;

  return ARRAY_SUCCESS;

CopyConstructorError:

  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}
