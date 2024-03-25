#define ARRAY_USE_IMPL
#include <array.h>

bool array_any(const Array * arr, const void * value) {
  return array_anyWithin(arr, 0, array_size(arr), value);
}

bool array_anyIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_anyWithinIf(arr, 0, array_size(arr), predFn, param);
}

int array_anyWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return ARRAY_FAILURE;

  for (ptrdiff_t i = from; i < to; ++i)
    if (!memcmp(array_cDataOffset(arr, i), value, array_dataSize(arr)))
      return true;

  return false;
}

int array_anyWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(predFn != NULL && "predicate function cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return ARRAY_FAILURE;

  for (size_t i = 0; i < array_size(arr); i++)
    if (predFn(arr, i, array_cDataOffset(arr, i), param))
      return true;

  return false;
}
