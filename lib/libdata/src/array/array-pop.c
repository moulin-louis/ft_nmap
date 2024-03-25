#define ARRAY_USE_IMPL
#include <array.h>

int array_pop(
    Array * arr, ptrdiff_t pos, size_t requestedCount, size_t * count
) {
  if (array_setupPos(arr, &pos))
    return ARRAY_FAILURE;

  requestedCount = min(requestedCount, array_size(arr) - pos);
  if (count)
    *count = requestedCount;

  array_getFactory(arr)->destructor(
      arr, array_dataOffset(arr, pos), requestedCount
  );
  memcpy(
      array_dataOffset(arr, pos),
      array_cDataOffset(arr, pos + requestedCount),
      (array_size(arr) - pos - requestedCount) * array_dataSize(arr)
  );
  array_setSize(arr, array_size(arr) - requestedCount);

  return ARRAY_SUCCESS;
}
