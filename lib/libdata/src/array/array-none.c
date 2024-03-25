#define ARRAY_USE_IMPL
#include <array.h>

bool array_none(const Array * arr, const void * value) {
  return array_noneWithin(arr, 0, array_size(arr), value);
}

bool array_noneIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_noneWithinIf(arr, 0, array_size(arr), predFn, param);
}

int array_noneWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  const int anyWithin = array_anyWithin(arr, from, to, value);
  return anyWithin == ARRAY_FAILURE ? ARRAY_FAILURE : !anyWithin;
}

int array_noneWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  const int anyWithinIf = array_anyWithinIf(arr, from, to, predFn, param);
  return anyWithinIf == ARRAY_FAILURE ? ARRAY_FAILURE : !anyWithinIf;
}
