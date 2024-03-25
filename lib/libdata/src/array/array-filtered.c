#define ARRAY_USE_IMPL
#include <array.h>

Array * array_filtered(const Array * arr, const void * value) {
  return array_filteredWithin(arr, 0, array_size(arr), value);
}

Array * array_filteredIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_filteredWithinIf(arr, 0, array_size(arr), predFn, param);
}

Array * array_filteredWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(value != NULL && "value cannot be NULL");

  Array * dst = NULL;

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE ||
      !(dst = array(
            array_dataSize(arr), array_size(arr), 0, NULL, array_getFactory(arr)
        )))
    goto AnyError;

  ptrdiff_t count;

  while (from < to) {
    count = 0;

    while (
        from + count < to &&
        memcmp(array_cDataOffset(arr, from + count), value, array_dataSize(arr))
    )
      ++count;
    if (count) {
      array_pushBack(dst, array_cDataOffset(arr, from), count);
      from += count;
    } else
      ++from;
  }
  if (array_shrink(dst))
    goto AnyError;

  return dst;

AnyError:

  array_destroy(dst);
  return NULL;
}

Array * array_filteredWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(predFn != NULL && "predicate function cannot be NULL");

  Array * dst = NULL;

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE ||
      !(dst = array(
            array_dataSize(arr), array_size(arr), 0, NULL, array_getFactory(arr)
        )))
    goto AnyError;

  ptrdiff_t count;

  while (from < to) {
    count = 0;

    while (
        from + count < to &&
        predFn(arr, from + count, array_cDataOffset(arr, from + count), param)
    )
      ++count;
    if (count) {
      array_pushBack(dst, array_cDataOffset(arr, from), count);
      from += count;
    } else
      ++from;
  }
  if (array_shrink(dst))
    goto AnyError;

  return dst;

AnyError:

  array_destroy(dst);
  return NULL;
}
