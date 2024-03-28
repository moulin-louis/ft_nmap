#define ARRAY_USE_IMPL
#include <array.h>

Array * array_replaced(
    const Array * arr, const void * oldValue, const void * newValue
) {
  return array_replacedWithin(arr, 0, array_size(arr), oldValue, newValue);
}

Array * array_replacedIf(
    const Array * arr,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
) {
  return array_replacedWithinIf(
      arr, 0, array_size(arr), predFn, param, newValue
  );
}

Array * array_replacedWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    const void * oldValue,
    const void * newValue
) {
  if (array_setupRange(arr, &from, &to))
    return NULL;

  Array * result = array(
      array_dataSize(arr), array_size(arr), 0, NULL, array_getFactory(arr)
  );
  if (!result)
    goto AnyError;

  ptrdiff_t count, i;
  size_t n;

  if (array_pushBack(result, array_cData(arr), from) == ARRAY_FAILURE)
    goto AnyError;

  while (from < to) {
    count = 0;
    i = 0;
    while (from + count < to && memcmp(
                                    array_dataOffset(arr, from + count),
                                    oldValue,
                                    array_dataSize(arr)
                                ))
      ++count;
    if (array_pushBack(result, array_cDataOffset(arr, from), count) ==
        ARRAY_FAILURE)
      goto AnyError;
    from += count;
    count = 0;
    while (from + count < to && !memcmp(
                                    array_cDataOffset(arr, from + count),
                                    oldValue,
                                    array_dataSize(arr)
                                ))
      ++count;
    if (array_pushBack(result, newValue, 1) == ARRAY_FAILURE)
      goto AnyError;
    i = 1;
    while (i < count) {
      n = min(i, count - i);
      if (array_pushBack(result, array_cDataOffset(result, from), n) ==
          ARRAY_FAILURE)
        goto AnyError;
      i += n;
    }
    from += count;
  }
  if (array_pushBack(
          result, array_cDataOffset(arr, from), array_size(arr) - to
      ) == ARRAY_FAILURE ||
      array_shrink(result) == ARRAY_FAILURE)
    goto AnyError;

  return result;

AnyError:

  array_destroy(result);
  return NULL;
}

Array * array_replacedWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
) {
  if (array_setupRange(arr, &from, &to))
    return NULL;

  Array * result = array(
      array_dataSize(arr), array_size(arr), 0, NULL, array_getFactory(arr)
  );
  if (!result)
    goto AnyError;

  size_t n;
  ptrdiff_t count, i;

  if (array_pushBack(result, array_cData(arr), from) == ARRAY_FAILURE)
    goto AnyError;
  while (from < to) {
    count = 0;
    i = 0;
    while (
        from + count < to &&
        !predFn(arr, from + count, array_cDataOffset(arr, from + count), param)
    )
      ++count;
    if (array_pushBack(result, array_cDataOffset(arr, from), count) ==
        ARRAY_FAILURE)
      goto AnyError;
    from += count;
    count = 0;
    while (from + count < to &&
           predFn(arr, from + count, array_dataOffset(arr, from + count), param)
    )
      ++count;
    if (array_pushBack(result, newValue, 1) == ARRAY_FAILURE)
      goto AnyError;
    i = 1;
    while (i < count) {
      n = min(i, count - i);
      if (array_pushBack(result, array_cDataOffset(result, from), n) ==
          ARRAY_FAILURE)
        goto AnyError;
      i += n;
    }
    from += count;
  }
  if (array_pushBack(
          result, array_cDataOffset(arr, from), array_size(arr) - to
      ) == ARRAY_FAILURE ||
      array_shrink(result) == ARRAY_FAILURE)
    goto AnyError;

  return result;

AnyError:

  array_destroy(result);
  return NULL;
}
