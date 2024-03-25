#define ARRAY_USE_IMPL
#include <array.h>

void * array_min(Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_minWithin(arr, 0, array_size(arr), cmpFn, param);
}

const void *
array_cMin(const Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_min((void *)arr, cmpFn, param);
}

void * array_minWithin(
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

  void * mini = array_dataOffset(arr, from);

  for (ptrdiff_t i = from + 1; i < to; ++i) {
    void * current = array_dataOffset(arr, i);
    if (cmpFn(current, mini, param) < 0)
      mini = current;
  }

  return mini;
}

const void * array_cMinWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  return array_minWithin((void *)arr, from, to, cmpFn, param);
}
