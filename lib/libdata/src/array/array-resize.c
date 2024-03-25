#define ARRAY_USE_IMPL
#include <array.h>

int array_resize(Array * arr, size_t size) {
  if (size == array_size(arr))
    return ARRAY_SUCCESS;

  if (array_reserve(arr, size))
    return ARRAY_FAILURE;
  if (size < array_size(arr)) {
    array_getFactory(arr)->destructor(
        arr, array_dataOffset(arr, size), array_size(arr) - size
    );
    array_setSize(arr, size);
  } else {
    if (array_getFactory(arr)->constructor(
            arr, array_dataOffset(arr, array_size(arr)), size - array_size(arr)
        ))
      goto ConstructorError;
    array_setSize(arr, size);
  }
  return ARRAY_SUCCESS;

ConstructorError:

  array_errno = ARR_ECTORFAIL;
  return ARRAY_FAILURE;
}
