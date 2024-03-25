#define ARRAY_USE_IMPL
#include <array.h>

int array_forEach(Array * arr, ArrayIterFunction * iterFn, void * param) {
  return array_forEachWithin(arr, 0, array_size(arr), iterFn, param);
}

int array_cForEach(
    const Array * arr, ArrayCIterFunction * iterFn, void * param
) {
  return array_forEach((void *)arr, (ArrayIterFunction *)iterFn, param);
}

int array_rForEach(Array * arr, ArrayIterFunction * iterFn, void * param) {
  return array_rForEachWithin(arr, 0, array_size(arr), iterFn, param);
}

int array_crForEach(
    const Array * arr, ArrayCIterFunction * iterFn, void * param
) {
  return array_rForEach((void *)arr, (ArrayIterFunction *)iterFn, param);
}

int array_forEachWithin(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayIterFunction * iterFn,
    void * param
) {
  assert(iterFn != NULL && "iterFn cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    goto AnyError;

  for (ptrdiff_t i = from; i < to; ++i)
    if (iterFn(arr, i, array_dataOffset(arr, i), param))
      goto IterFunctionError;

  return ARRAY_SUCCESS;

IterFunctionError:

  array_errno = ARR_EITERFAIL;

AnyError:

  return ARRAY_FAILURE;
}

int array_cForEachWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCIterFunction * iterFn,
    void * param
) {
  return array_forEachWithin(
      (void *)arr, from, to, (ArrayIterFunction *)iterFn, param
  );
}

int array_rForEachWithin(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayIterFunction * iterFn,
    void * param
) {
  assert(iterFn != NULL && "iterFn cannot be NULL");

  for (ptrdiff_t i = to; i > from; --i)
    if (iterFn(arr, i - 1, array_dataOffset(arr, i - 1), param))
      goto IterFunctionError;

  return ARRAY_SUCCESS;

IterFunctionError:

  array_errno = ARR_EITERFAIL;
  return ARRAY_FAILURE;
}

int array_crForEachWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCIterFunction * iterFn,
    void * param
) {
  return array_rForEachWithin(
      (void *)arr, from, to, (ArrayIterFunction *)iterFn, param
  );
}
