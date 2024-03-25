#define ARRAY_USE_IMPL
#include <array.h>

int array_extend(Array * arr, const Array * other) {
  if (array_dataSize(arr) != array_dataSize(other))
    goto DataSizesNotSameError;

  if (array_grow(arr, array_size(arr) + array_size(other)))
    goto AnyError;

  if (array_getFactory(arr)->copyConstructor(
          arr,
          array_dataOffset(arr, array_size(arr)),
          array_cData(other),
          array_size(other)
      ))
    goto CopyConstructorError;
  array_setSize(arr, array_size(arr) + array_size(other));

  return ARRAY_SUCCESS;

DataSizesNotSameError:

  array_errno = ARR_EDIFFDATASIZE;
  goto AnyError;

CopyConstructorError:

  array_errno = ARR_ECPYCTORFAIL;
  goto AnyError;

AnyError:

  return ARRAY_FAILURE;
}
