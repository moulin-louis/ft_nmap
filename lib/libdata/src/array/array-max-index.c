#define ARRAY_USE_IMPL
#include <array.h>

ptrdiff_t
array_maxIndex(const Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_maxIndexWithin(arr, 0, array_size(arr), cmpFn, param);
}

ptrdiff_t array_maxIndexWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  assert(cmpFn != NULL && "compare function cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  ptrdiff_t maxIdx = from;
  for (ptrdiff_t i = from + 1; i < to; ++i) {
    if (cmpFn(
            array_cDataOffset(arr, i), array_cDataOffset(arr, maxIdx), param
        ) > 0)
      maxIdx = i;
  }
  return maxIdx;
}
