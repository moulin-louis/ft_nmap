#define ARRAY_USE_IMPL
#include <array.h>

static int array_mergeSort(
    Array * arr,
    size_t pos,
    size_t n,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  if (n < 2)
    return 0;

  const size_t leftSize = n / 2;
  const size_t rightSize = n - n / 2;

  if (array_mergeSort(arr, pos, leftSize, cmpFn, param) ||
      array_mergeSort(arr, pos + leftSize, rightSize, cmpFn, param))
    goto AnyError;

  uint8_t * dst = malloc(n * array_dataSize(arr));
  if (!dst)
    goto NoMemoryError;

  size_t leftIndex, rightIndex, dstIndex;

  leftIndex = rightIndex = dstIndex = 0;
  while (leftIndex < leftSize && rightIndex < rightSize) {
    int cmpRet;
    size_t count = 0;

    if (rightIndex < rightSize)
      while (leftIndex + count < leftSize &&
             (cmpRet = cmpFn(
                  array_cDataOffset(arr, pos + leftIndex + count),
                  array_cDataOffset(arr, pos + leftSize + rightIndex),
                  param
              )) < 0)
        ++count;
    memcpy(
        dst + dstIndex * array_dataSize(arr),
        array_cDataOffset(arr, pos + leftIndex),
        count * array_dataSize(arr)
    );
    dstIndex += count;
    leftIndex += count;
    count = 0;
    if (leftIndex < leftSize)
      while (rightIndex + count < rightSize && cmpRet >= 0) {
        ++count;
        cmpRet = cmpFn(
            array_cDataOffset(arr, pos + leftIndex),
            array_cDataOffset(arr, pos + leftSize + rightIndex + count),
            param
        );
      }
    memcpy(
        dst + dstIndex * array_dataSize(arr),
        array_cDataOffset(arr, pos + leftSize + rightIndex),
        count * array_dataSize(arr)
    );
    dstIndex += count;
    rightIndex += count;
  }
  memcpy(
      dst + dstIndex * array_dataSize(arr),
      array_cDataOffset(arr, pos + leftIndex),
      (leftSize - leftIndex) * array_dataSize(arr)
  );
  memcpy(
      dst + dstIndex * array_dataSize(arr),
      array_cDataOffset(arr, pos + leftSize + rightIndex),
      (rightSize - rightIndex) * array_dataSize(arr)
  );
  memcpy(array_dataOffset(arr, pos), dst, n * array_dataSize(arr));
  free(dst);
  return ARRAY_SUCCESS;

NoMemoryError:

  array_errno = ARR_ENOMEM;

AnyError:

  return ARRAY_FAILURE;
}

int array_sort(Array * arr, ArrayCompareFunction * cmpFn, void * param) {
  return array_sortWithin(arr, 0, array_size(arr), cmpFn, param);
}

int array_sortWithin(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
) {
  assert(cmpFn != NULL && "compare function cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  return array_mergeSort(arr, from, to - from, cmpFn, param);
}
