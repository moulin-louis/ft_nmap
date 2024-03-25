#define ARRAY_USE_IMPL
#include <array.h>

int array_replace(Array * arr, const void * oldValue, const void * newValue) {
  return array_replaceWithin(arr, 0, array_size(arr), oldValue, newValue);
}

int array_replaceIf(
    Array * arr,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
) {
  return array_replaceWithinIf(
      arr, 0, array_size(arr), predFn, param, newValue
  );
}

int array_replaceWithin(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    const void * oldValue,
    const void * newValue
) {
  if (array_setupRange(arr, &from, &to))
    goto AnyError;

  ptrdiff_t i;
  size_t n;
  while (from < to) {
    ptrdiff_t count = 0;

    while (from + count < to && !memcmp(
                                    array_cDataOffset(arr, from + count),
                                    oldValue,
                                    array_dataSize(arr)
                                ))
      ++count;

    i = 0;
    n = 0;
    if (count) {
      array_getFactory(arr)->destructor(
          arr, array_dataOffset(arr, from), count
      );

      if (array_getFactory(arr)->copyConstructor(
              arr, array_dataOffset(arr, from), newValue, 1
          ))
        goto CopyConstructorError;
      i = 1;
      while (i < count) {
        n = min(i, count - i);
        if (array_getFactory(arr)->copyConstructor(
                arr,
                array_dataOffset(arr, from + i),
                array_cDataOffset(arr, from),
                n
            ))
          goto CopyConstructorError;
        i += n;
      }
      from += count;
    } else
      ++from;
  }

  return ARRAY_SUCCESS;

CopyConstructorError:

  array_getFactory(arr)->destructor(arr, array_dataOffset(arr, from), i);
  memcpy(
      array_dataOffset(arr, from),
      array_cDataOffset(arr, from + i + n),
      (array_size(arr) - from - i - n) * array_dataSize(arr)
  );
  array_setSize(arr, array_size(arr) - i - n);
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}

int array_replaceWithinIf(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
) {
  if (array_setupRange(arr, &from, &to))
    goto AnyError;

  ptrdiff_t i;
  size_t n;
  while (from < to) {
    ptrdiff_t count = 0;

    while (from + count < to &&
           predFn(arr, from + count, array_dataOffset(arr, from + count), param)
    )
      ++count;

    if (count) {
      i = 0;
      n = 1;
      array_getFactory(arr)->destructor(
          arr, array_dataOffset(arr, from), count
      );

      if (array_getFactory(arr)->copyConstructor(
              arr, array_dataOffset(arr, from), newValue, n
          ))
        goto CopyConstructorError;
      i = 1;
      while (i < count) {
        n = min(i, count - i);
        if (array_getFactory(arr)->copyConstructor(
                arr,
                array_dataOffset(arr, from + i),
                array_cDataOffset(arr, from),
                n
            ))
          goto CopyConstructorError;
        i += n;
      }
      from += count;
    } else
      ++from;
  }

  return ARRAY_SUCCESS;

CopyConstructorError:

  memcpy(
      array_dataOffset(arr, from + i),
      array_cDataOffset(arr, from + i + n),
      (array_size(arr) - from - i - n) * array_dataSize(arr)
  );
  array_setSize(arr, array_size(arr) - i - n);
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}
