#define ARRAY_USE_IMPL
#include <array.h>

int array_compare(
    const Array * lhs,
    const Array * rhs,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  assert(cmpFn != NULL && "cmpFn cannot be NULL");

  const size_t n = min(array_size(lhs), array_size(rhs));
  size_t i = 0;
  while (i < n) {
    int cmpRet =
        cmpFn(array_cDataOffset(lhs, i), array_cDataOffset(rhs, i), param);
    if (cmpRet)
      return cmpRet;
    ++i;
  }
  return array_size(lhs) == array_size(rhs) ? 0 : i == array_size(lhs) ? -1 : 1;
}

int array_compareBytes(const Array * lhs, const Array * rhs) {
  const size_t lhsBytes = array_size(lhs) * array_dataSize(lhs);
  const size_t rhsBytes = array_size(rhs) * array_dataSize(rhs);

  size_t i = 0;
  int cmpRet =
      memcmp(array_cData(lhs), array_cData(rhs), min(lhsBytes, rhsBytes));
  if (cmpRet || lhsBytes == rhsBytes)
    return cmpRet;
  return i == lhsBytes ? -1 : 1;
}

int array_compareData(
    const Array * lhs,
    const void * rhs,
    size_t size,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  assert(cmpFn != NULL && "cmpFn cannot be NULL");

  const uint8_t * rhsData = rhs;
  const size_t n = min(array_size(lhs), size);
  size_t i = 0;
  while (i < n) {
    int cmpRet = cmpFn(
        array_cDataOffset(lhs, i), rhsData + i * array_dataSize(lhs), param
    );
    if (cmpRet)
      return cmpRet;
    ++i;
  }
  return array_size(lhs) == size ? 0 : i == array_size(lhs) ? -1 : 1;
}
