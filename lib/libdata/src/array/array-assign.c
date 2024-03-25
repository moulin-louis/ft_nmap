#define ARRAY_USE_IMPL
#include <array.h>

int array_assign(Array * arr, const Array * other) {
  size_t minCapacity = ceil(
      array_size(other) * array_dataSize(other) / (double)array_dataSize(arr)
  );

  if (arr == other)
    return 0;

  array_clear(arr);
  array_setFactory(arr, array_getFactory(other));
  if (array_grow(arr, minCapacity))
    goto AnyError;
  array_setSize(arr, array_size(other));
  if (array_getFactory(arr)->copyConstructor(
          arr, array_data(arr), array_cData(other), array_size(other)
      ))
    goto CopyConstructorError;
  array_setSize(arr, array_size(other));
  return ARRAY_SUCCESS;

CopyConstructorError:

  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}

int array_assignData(Array * arr, const void * data, size_t count) {
  assert(data != NULL && "data cannot be NULL");

  array_clear(arr);
  if (array_grow(arr, count))
    goto AnyError;

  if (array_getFactory(arr)->copyConstructor(arr, array_data(arr), data, count))
    goto CopyConstructorError;
  array_setSize(arr, count);

  return ARRAY_SUCCESS;

CopyConstructorError:

  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  return ARRAY_FAILURE;
}
