#define ARRAY_USE_IMPL
#include <array.h>

Array *
array_sorted(const Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_sortedWithin(arr, 0, array_size(arr), cmpFn, param);
}

Array * array_sortedWithin(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  assert(cmpFn != NULL && "compare function cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return NULL;

  Array * dst = array_clone(arr);
  if (!dst)
    return NULL;

  if (array_sortWithin(dst, from, to, cmpFn, param)) {
    array_destroy(dst);
    dst = NULL;
  }
  return dst;
}
