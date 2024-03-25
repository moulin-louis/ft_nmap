#define ARRAY_IMPL
#include <array.h>

int array_shrink(Array * arr) {
  if (array_size(arr) == array_capacity(arr))
    return ARRAY_SUCCESS;

  uint8_t * tmp = arr->factory.reallocator(
      arr->data, array_size(arr) * array_dataSize(arr)
  );
  if (!tmp)
    goto NoMemoryError;

  arr->data = tmp;
  array_setCapacity(arr, array_size(arr));
  return ARRAY_SUCCESS;

NoMemoryError:

  array_errno = ARR_ENOMEM;
  return ARRAY_FAILURE;
}
