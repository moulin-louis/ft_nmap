#define ARRAY_USE_IMPL
#include <array.h>

int array_count(const Array * arr, const void * value) {
  return array_countWithin(arr, 0, array_size(arr), value);
}

int array_countIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_countWithinIf(arr, 0, array_size(arr), predFn, param);
}

int array_countWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return ARRAY_FAILURE;

  int count = 0;
  for (ptrdiff_t i = from; i < to; ++i)
    if (!memcmp(array_cDataOffset(arr, i), value, array_dataSize(arr)))
      ++count;

  return count;
}

int array_countWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(predFn != NULL && "predicate function cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return ARRAY_FAILURE;

  int count = 0;
  for (ptrdiff_t i = from; i < to; i++)
    if (predFn(arr, i, array_cDataOffset(arr, i), param))
      ++count;

  return count;
}
