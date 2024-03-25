#define ARRAY_USE_IMPL
#include <array.h>

void * array_reduce(
    Array * arr,
    size_t dataSize,
    const void * init,
    ArrayMapFunction * mapFn,
    void * param
) {
  assert(mapFn != NULL && "mapFn cannot be NULL");

  void * result = malloc(dataSize);
  if (!result)
    goto NoMemoryError;
  if (init)
    memcpy(result, init, dataSize);
  else
    memset(result, 0, dataSize);

  for (size_t i = 0; i < array_size(arr); ++i)
    if (mapFn(arr, i, result, array_dataOffset(arr, i), param))
      goto MapFunctionError;

  return result;

NoMemoryError:

  array_errno = ARR_ENOMEM;
  goto AnyError;

MapFunctionError:

  free(result);
  array_errno = ARR_EMAPFAIL;

AnyError:

  return NULL;
}

void * array_cReduce(
    const Array * arr,
    size_t dataSize,
    const void * init,
    ArrayCMapFunction * mapFn,
    void * param
) {
  return array_reduce(
      (void *)arr, dataSize, init, (ArrayMapFunction *)mapFn, param
  );
}

void * array_rReduce(
    Array * arr,
    size_t dataSize,
    const void * init,
    ArrayMapFunction * mapFn,
    void * param
) {
  assert(mapFn != NULL && "mapFn cannot be NULL");

  void * result = malloc(dataSize);
  if (!result)
    goto NoMemoryError;
  if (init)
    memcpy(result, init, dataSize);
  else
    memset(result, 0, dataSize);

  for (size_t i = array_size(arr); i > 0; --i)
    if (mapFn(arr, i - 1, result, array_dataOffset(arr, i - 1), param))
      goto MapFunctionError;

  return result;

NoMemoryError:

  array_errno = ARR_ENOMEM;
  goto AnyError;

MapFunctionError:

  free(result);
  array_errno = ARR_EMAPFAIL;

AnyError:

  return NULL;
}

void * array_crReduce(
    const Array * arr,
    size_t dataSize,
    const void * init,
    ArrayCMapFunction * mapFn,
    void * param
) {
  return array_rReduce(
      (void *)arr, dataSize, init, (ArrayMapFunction *)mapFn, param
  );
}
