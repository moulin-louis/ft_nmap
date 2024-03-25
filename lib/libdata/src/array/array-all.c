#define ARRAY_USE_IMPL
#include <array.h>

bool array_all(const Array * arr, const void * value) {
  return array_allWithin(arr, 0, array_size(arr), value);
}

bool array_allIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_allWithinIf(arr, 0, array_size(arr), predFn, param);
}

int array_allWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return ARRAY_FAILURE;

  for (ptrdiff_t i = from; i < to; ++i)
    if (memcmp(array_cDataOffset(arr, i), value, array_dataSize(arr)))
      return false;

  return true;
}

int array_allWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(predFn != NULL && "predicate function cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return ARRAY_FAILURE;

  for (ptrdiff_t i = from; i < to; ++i)
    if (!predFn(arr, i, array_cDataOffset(arr, i), param))
      return false;

  return true;
}
