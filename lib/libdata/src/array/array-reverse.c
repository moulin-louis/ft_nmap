#define ARRAY_IMPL
#include <array.h>

void array_reverse(Array * arr) {
  assert(arr != NULL && "array cannot be NULL");

  array_reverseWithin(arr, 0, arr->size);
}

int array_reverseWithin(Array * arr, ptrdiff_t from, ptrdiff_t to) {
  assert(arr != NULL && "array cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return ARRAY_FAILURE;

  while (from < --to) {
    memswap(
        arr->data + from * arr->dataSize,
        arr->data + to * arr->dataSize,
        arr->dataSize
    );
    ++from;
  }

  return ARRAY_SUCCESS;
}
