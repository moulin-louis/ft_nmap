#define ARRAY_USE_IMPL
#include <array.h>

ptrdiff_t
array_minIndex(const Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_minIndexWithin(arr, 0, array_size(arr), cmpFn, param);
}

ptrdiff_t array_minIndexWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  assert(cmpFn != NULL && "compare function cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  ptrdiff_t minIndex = from;
  for (ptrdiff_t i = from + 1; i < to; ++i) {
    if (cmpFn(
            array_dataOffset(arr, i), array_dataOffset(arr, minIndex), param
        ) < 0)
      minIndex = i;
  }
  return minIndex;
}
