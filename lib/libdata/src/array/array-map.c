#define ARRAY_USE_IMPL
#include <array.h>

Array * array_map(
    Array * arr,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayMapFunction * mapFn,
    void * param
) {
  assert(arr != NULL && "array cannot be NULL");
  assert(mapFn != NULL && "mapFn cannot be NULL");

  Array * dst =
      array(dataSize, array_size(arr), 0, NULL, factory ? factory : NULL);

  if (!dst)
    goto AnyError;

  for (size_t i = 0; i < array_size(arr); ++i) {
    if (mapFn(
            arr, i, array_dataOffset(dst, i), array_dataOffset(arr, i), param
        ))
      goto MapFunctionError;
  }
  array_setSize(dst, array_size(arr));

  return dst;

MapFunctionError:

  array_destroy(dst);
  array_errno = ARR_EMAPFAIL;

AnyError:
  return NULL;
}

Array * array_cMap(
    const Array * arr,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayCMapFunction * mapFn,
    void * param
) {
  return array_map(
      (void *)arr, dataSize, factory, (ArrayMapFunction *)mapFn, param
  );
}

Array * array_rMap(
    Array * arr,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayMapFunction * mapFn,
    void * param
) {
  assert(arr != NULL && "array cannot be NULL");
  assert(mapFn != NULL && "mapFn cannot be NULL");

  Array * dst =
      array(dataSize, array_size(arr), 0, NULL, factory ? factory : NULL);

  if (!dst)
    goto AnyError;

  for (size_t i = array_size(arr); i > 0; --i) {
    if (mapFn(
            arr,
            i - 1,
            array_dataOffset(dst, i - 1),
            array_dataOffset(arr, i - 1),
            param
        ))
      goto MapFunctionError;
  }
  array_setSize(dst, array_size(arr));

  return dst;

MapFunctionError:

  array_destroy(dst);
  array_errno = ARR_EMAPFAIL;

AnyError:

  return NULL;
}

Array * array_crMap(
    const Array * arr,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayCMapFunction * mapFn,
    void * param
) {
  return array_rMap(
      (void *)arr, dataSize, factory, (ArrayMapFunction *)mapFn, param
  );
}
