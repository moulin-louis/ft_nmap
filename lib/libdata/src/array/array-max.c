#define ARRAY_USE_IMPL
#include <array.h>

void * array_max(Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_maxWithin(arr, 0, array_size(arr), cmpFn, param);
}

const void *
array_cMax(const Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_max((void *)arr, cmpFn, param);
}

void * array_maxWithin(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  if (array_setupRange(arr, &from, &to))
    return NULL;

  if (from == to)
    return NULL;

  void * maxi = array_dataOffset(arr, from);

  for (ptrdiff_t i = from + 1; i < to; ++i) {
    void * current = array_dataOffset(arr, i);
    if (cmpFn(current, maxi, param) > 0)
      maxi = current;
  }

  return maxi;
}

const void * array_cMaxWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  return array_maxWithin((void *)arr, from, to, cmpFn, param);
}
