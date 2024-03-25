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

  Array * result =
      array(array_dataSize(arr), to - from, 0, NULL, array_getFactory(arr));
  if (!result)
    goto AnyError;

  size_t resultIdx = 0;
  ptrdiff_t count, i;
  size_t n;

  while (from < to) {
    count = 0;
    i = 0;
    while (from + count < to && memcmp(
                                    array_dataOffset(arr, from + count),
                                    oldValue,
                                    array_dataSize(arr)
                                ))
      ++count;
    if (array_getFactory(result)->copyConstructor(
            result,
            array_dataOffset(result, resultIdx),
            array_cDataOffset(arr, from),
            count
        ))
      goto CopyConstructorError;
    resultIdx += count;
    from += count;
    count = 0;
    while (from + count < to && !memcmp(
                                    array_cDataOffset(arr, from + count),
                                    oldValue,
                                    array_dataSize(arr)
                                ))
      ++count;
    if (count) {
      if (array_getFactory(result)->copyConstructor(
              result, array_dataOffset(result, resultIdx), newValue, 1
          ))
        goto CopyConstructorError;
      i = 1;
      while (i < count) {
        n = min(i, count - i);
        if (array_getFactory(result)->copyConstructor(
                result,
                array_dataOffset(result, resultIdx + i),
                array_cDataOffset(result, resultIdx),
                n
            ))
          goto CopyConstructorError;
        i += n;
      }
      resultIdx += count;
      from += count;
    } else
      ++from;
  }
  array_setSize(result, resultIdx);
  if (array_shrink(result) == ARRAY_FAILURE)
    goto AnyError;

  return result;

CopyConstructorError:

  array_getFactory(result)->destructor(
      result, array_data(result), resultIdx + i
  );
  array_errno = ARR_ECPYCTORFAIL;

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

  Array * result =
      array(array_dataSize(arr), to - from, 0, NULL, array_getFactory(arr));
  if (!result)
    goto AnyError;

  size_t resultIdx = 0;
  size_t n;
  ptrdiff_t count, i;

  while (from < to) {
    count = 0;
    i = 0;
    while (
        from + count < to &&
        !predFn(arr, from + count, array_cDataOffset(arr, from + count), param)
    )
      ++count;
    if (array_getFactory(result)->copyConstructor(
            result,
            array_dataOffset(result, resultIdx),
            array_cDataOffset(arr, from),
            count
        ))
      goto CopyConstructorError;
    resultIdx += count;
    from += count;
    count = 0;
    while (from + count < to &&
           predFn(arr, from + count, array_dataOffset(arr, from + count), param)
    )
      ++count;
    if (count) {
      if (array_getFactory(result)->copyConstructor(
              result, array_dataOffset(result, resultIdx), newValue, 1
          ))
        goto CopyConstructorError;
      i = 1;
      while (i < count) {
        n = min(i, count - i);
        if (array_getFactory(result)->copyConstructor(
                result,
                array_dataOffset(result, resultIdx + i),
                array_cDataOffset(result, resultIdx),
                n
            ))
          goto CopyConstructorError;
        i += n;
      }
      resultIdx += count;
      from += count;
    } else
      ++from;
  }
  array_setSize(result, resultIdx);
  if (array_shrink(result) == ARRAY_FAILURE)
    goto AnyError;

  return result;

CopyConstructorError:

  array_getFactory(result)->destructor(
      result, array_data(result), resultIdx + i
  );
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  array_destroy(result);
  return NULL;
}
