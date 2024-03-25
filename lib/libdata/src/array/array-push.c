#define ARRAY_USE_IMPL
#include <array.h>

#include <stdio.h>
int array_push(Array * arr, ptrdiff_t pos, const void * values, size_t count) {
  assert(values != NULL && "values cannot be NULL");

  const size_t minCapacity = array_size(arr) + count;

  if (array_setupPos(arr, &pos) || array_grow(arr, minCapacity))
    goto AnyError;

  memcpy(
      array_dataOffset(arr, pos + count),
      array_cDataOffset(arr, pos),
      (array_size(arr) - pos) * array_dataSize(arr)
  );
  if (array_getFactory(arr)->copyConstructor(
          arr, array_dataOffset(arr, pos), values, count
      ))
    goto CopyConstructorError;
  array_setSize(arr, array_size(arr) + count);

  return ARRAY_SUCCESS;

CopyConstructorError:

  memcpy(
      array_dataOffset(arr, pos),
      array_cDataOffset(arr, pos + count),
      (array_size(arr) - pos) * array_dataSize(arr)
  );
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}
