#define ARRAY_USE_IMPL
#include <array.h>

int array_emplace(Array * arr, ptrdiff_t pos, size_t count) {
  if (array_setupPos(arr, &pos))
    goto AnyError;

  const size_t minCapacity = array_size(arr) + count;

  if (array_grow(arr, minCapacity))
    goto AnyError;

  memmove(
      array_dataOffset(arr, pos + count),
      array_cDataOffset(arr, pos),
      (array_size(arr) - pos) * array_dataSize(arr)
  );
  if (array_getFactory(arr)->constructor(
          arr, array_dataOffset(arr, pos), count
      ))
    goto ConstructorError;

  array_setSize(arr, array_size(arr) + count);
  return ARRAY_SUCCESS;

ConstructorError:

  memcpy(
      array_dataOffset(arr, pos),
      array_cDataOffset(arr, pos + count),
      (array_size(arr) - pos) * array_dataSize(arr)
  );
  array_errno = ARR_ECTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}
