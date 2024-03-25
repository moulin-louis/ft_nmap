#define ARRAY_IMPL
#include <array.h>

void * array_find(Array * arr, const void * value) {
  assert(arr != NULL && "array cannot be NULL");

  return array_findWithin(arr, 0, arr->size, value);
}

const void * array_cFind(const Array * arr, const void * value) {
  return array_find((void *)arr, value);
}

void * array_rFind(Array * arr, const void * value) {
  assert(arr != NULL && "array cannot be NULL");

  return array_rFindWithin(arr, 0, arr->size, value);
}

const void * array_crFind(const Array * arr, const void * value) {
  return array_rFind((void *)arr, value);
}

void *
array_findIf(Array * arr, ArrayPredicateFunction * predFn, void * param) {
  assert(arr != NULL && "array cannot be NULL");

  return array_findWithinIf(arr, 0, arr->size, predFn, param);
}

const void * array_cFindIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_findIf((void *)arr, predFn, param);
}

void *
array_rFindIf(Array * arr, ArrayPredicateFunction * predFn, void * param) {
  return array_rFindWithinIf(arr, 0, arr->size, predFn, param);
}

const void * array_crFindIf(
    const Array * arr, ArrayPredicateFunction * predFn, void * param
) {
  return array_rFindIf((void *)arr, predFn, param);
}

void * array_findWithin(
    Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(arr != NULL && "array cannot be NULL");
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return NULL;

  for (ptrdiff_t i = from; i < to; ++i)
    if (!memcmp(arr->data + i * arr->dataSize, value, arr->dataSize))
      return arr->data + i * arr->dataSize;

  return NULL;
}

const void * array_cFindWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  return array_findWithin((void *)arr, from, to, value);
}

void * array_rFindWithin(
    Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  assert(arr != NULL && "array cannot be NULL");
  assert(value != NULL && "value cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return NULL;

  for (ptrdiff_t i = to - 1; i >= from; --i)
    if (!memcmp(arr->data + i * arr->dataSize, value, arr->dataSize))
      return arr->data + i * arr->dataSize;

  return NULL;
}

const void * array_crFindWithin(
    const Array * arr, ptrdiff_t from, ptrdiff_t to, const void * value
) {
  return array_rFindWithin((void *)arr, from, to, value);
}

void * array_findWithinIf(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(arr != NULL && "array cannot be NULL");
  assert(predFn != NULL && "predicate function cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return NULL;

  for (ptrdiff_t i = from; i < to; ++i)
    if (predFn(arr, i, arr->data + i * arr->dataSize, param))
      return arr->data + i * arr->dataSize;

  return NULL;
}

const void * array_cFindWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  return array_findWithinIf((void *)arr, from, to, predFn, param);
}

void * array_rFindWithinIf(
    Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  assert(arr != NULL && "array cannot be NULL");
  assert(predFn != NULL && "predicate function cannot be NULL");

  if (array_setupRange(arr, &from, &to) == ARRAY_FAILURE)
    return NULL;

  for (ptrdiff_t i = to - 1; i >= from; --i)
    if (predFn(arr, i, arr->data + i * arr->dataSize, param))
      return arr->data + i * arr->dataSize;

  return NULL;
}

const void * array_crFindWithinIf(
    const Array * arr,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
) {
  return array_rFindWithinIf((void *)arr, from, to, predFn, param);
}
