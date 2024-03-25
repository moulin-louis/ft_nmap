#define ARRAY_USE_IMPL
#include <array.h>

Array * array_rotated(const Array * arr, ptrdiff_t n) {
  return array_rotatedWithin(arr, 0, array_size(arr), n);
}

#include <stdio.h>
Array * array_rotatedWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, ptrdiff_t n
) {
  if (array_setupRange(arr, &from, &to))
    return NULL;

  Array * rotated = array(
      array_dataSize(arr), array_size(arr), 0, NULL, array_getFactory(arr)
  );
  if (!rotated)
    goto AnyError;

  const size_t size = to - from;
  const size_t absN = llabs(n) % size;

  n = n < 0 ? -absN : absN;
  if (n < 0)
    n += size;

  size_t i = 0;

  if (array_getFactory(rotated)->copyConstructor(
          rotated, array_data(rotated), array_cData(arr), from
      ))
    goto CopyConstructorError;

  i = from;

  if (array_getFactory(rotated)->copyConstructor(
          rotated,
          array_dataOffset(rotated, from),
          array_cDataOffset(arr, from + n),
          (size - n)
      ))
    goto CopyConstructorError;

  i += (size - n);

  if (array_getFactory(rotated)->copyConstructor(
          rotated,
          array_dataOffset(rotated, from + size - n),
          array_cDataOffset(arr, from),
          n
      ))
    goto CopyConstructorError;

  i += n;

  if (array_getFactory(rotated)->copyConstructor(
          rotated,
          array_dataOffset(rotated, to),
          array_cDataOffset(arr, to),
          (array_size(arr) - to)
      ))
    goto CopyConstructorError;

  array_setSize(rotated, array_size(arr));

  return rotated;

CopyConstructorError:

  array_getFactory(rotated)->destructor(rotated, array_data(rotated), i);
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  array_destroy(rotated);
  return NULL;
}
