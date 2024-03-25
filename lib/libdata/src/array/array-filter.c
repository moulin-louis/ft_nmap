#define ARRAY_USE_IMPL
#include <array.h>

void array_filter(Array * arr, const void * value) {
  array_filterWithin(arr, 0, array_size(arr), value);
}

void array_filterIf(
    Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  array_filterWithinIf(arr, 0, array_size(arr), predFn, param);
}

int array_filterWithin(
    Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  while (from < to) {
    ptrdiff_t count = 0;

    while (from + count < to &&
           !memcmp(
               array_cDataOffset(arr, from + count), value, array_dataSize(arr)
           ))
      ++count;
    if (count) {
      array_getFactory(arr)->destructor(
          arr, array_dataOffset(arr, from), count
      );
      memcpy(
          array_dataOffset(arr, from),
          array_cDataOffset(arr, from + count),
          (array_size(arr) - from - count) * array_dataSize(arr)
      );
      array_setSize(arr, array_size(arr) - count);
      to -= count;
    } else
      ++from;
  }
  return ARRAY_SUCCESS;
}

int array_filterWithinIf(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(predFn != NULL && "predicate function cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  while (from < to) {
    ptrdiff_t count = 0;

    while (
        from + count < to &&
        !predFn(arr, from + count, array_cDataOffset(arr, from + count), param)
    )
      ++count;
    if (count) {
      array_getFactory(arr)->destructor(
          arr, array_dataOffset(arr, from), count
      );
      memcpy(
          array_dataOffset(arr, from),
          array_cDataOffset(arr, from + count),
          (array_size(arr) - from - count) * array_dataSize(arr)
      );
      array_setSize(arr, array_size(arr) - count);
      to -= count;
    } else
      ++from;
  }
  return ARRAY_SUCCESS;
}
