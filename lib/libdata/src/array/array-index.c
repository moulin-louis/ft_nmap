#define ARRAY_USE_IMPL
#include <array.h>

ptrdiff_t array_index(const Array * arr, const void * value) {
  assert(arr != NULL && "array cannot be NULL");

  return array_indexWithin(arr, 0, array_size(arr), value);
}

ptrdiff_t array_rIndex(const Array * arr, const void * value) {
  assert(arr != NULL && "array cannot be NULL");

  return array_rIndexWithin(arr, 0, array_size(arr), value);
}

ptrdiff_t array_indexIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  assert(arr != NULL && "array cannot be NULL");

  return array_indexWithinIf(arr, 0, array_size(arr), predFn, param);
}

ptrdiff_t array_rIndexIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  assert(arr != NULL && "array cannot be NULL");

  return array_rIndexWithinIf(arr, 0, array_size(arr), predFn, param);
}

ptrdiff_t array_indexWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  const uint8_t * found = array_cFindWithin(arr, from, to, value);

  if (found)
    return (found - (uint8_t *)array_cData(arr)) / array_dataSize(arr);
  return -1;
}

ptrdiff_t array_rIndexWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  const uint8_t * found = array_crFindWithin(arr, from, to, value);

  if (found)
    return (found - (uint8_t *)array_cData(arr)) / array_dataSize(arr);
  return -1;
}

ptrdiff_t array_indexWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  const uint8_t * found = array_cFindWithinIf(arr, from, to, predFn, param);

  if (found)
    return (found - (uint8_t *)array_cData(arr)) / array_dataSize(arr);
  return -1;
}

ptrdiff_t array_rIndexWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  const uint8_t * found = array_crFindWithinIf(arr, from, to, predFn, param);

  if (found)
    return (found - (uint8_t *)array_cData(arr)) / array_dataSize(arr);
  return -1;
}
