#define ARRAY_USE_IMPL
#include <array.h>

int array_reserve(Array * arr, size_t capacity) {
  if (capacity <= array_capacity(arr))
    return ARRAY_SUCCESS;

  void * data = array_getFactory(arr)->reallocator(
      array_data(arr), capacity * array_dataSize(arr)
  );
  if (data == NULL)
    goto NoMemoryError;

  array_setData(arr, data);
  array_setCapacity(arr, capacity);

  return ARRAY_SUCCESS;

NoMemoryError:

  array_errno = ARR_ENOMEM;
  return ARRAY_FAILURE;
}
