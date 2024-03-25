#include <stdio.h>

#define ARRAY_USE_IMPL
#include "inc/array.h"
#include "inc/tools.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>

size_t allocatedBlocks;
size_t freedBlocks;
size_t defaultConstructedElements;
size_t copyConstructedElements;
size_t destroyedElements;

void * customAllocator(size_t size) {
  ++allocatedBlocks;
  return malloc(size);
}

void customDeallocator(void * ptr) {
  ++freedBlocks;
  free(ptr);
}

int customConstructor(Array * arr, void * data, size_t n) {
  defaultConstructedElements += n;
  return array_defaultCtor(arr, data, n);
}

int customCopyConstructor(Array * arr, void * dst, const void * src, size_t n) {
  copyConstructedElements += n;
  return array_defaultCopyCtor(arr, dst, src, n);
}

void customDestructor(Array * arr, void * data, size_t n) {
  destroyedElements += n;
  array_defaultDtor(arr, data, n);
}

const ArrayFactory customFactory = {
    .allocator = customAllocator,
    .deallocator = customDeallocator,
    .constructor = customConstructor,
    .copyConstructor = customCopyConstructor,
    .destructor = customDestructor,
};

void init(void) {
  array_errno = ARR_SUCCESS;
  allocatedBlocks = 0;
  freedBlocks = 0;
  defaultConstructedElements = 0;
  copyConstructedElements = 0;
  destroyedElements = 0;
}

void fini_assert(void) {
  // printf("allocated: %zu, freed: %zu\n", allocatedBlocks, freedBlocks);
  // printf(
  //     "default constructed: %zu, copy constructed: %zu, destroyed: %zu\n",
  //     defaultConstructedElements,
  //     copyConstructedElements,
  //     destroyedElements
  // );
  cr_assert(eq(sz, allocatedBlocks, freedBlocks));
  cr_assert(
      eq(sz,
         defaultConstructedElements + copyConstructedElements,
         destroyedElements)
  );
}

TestSuite(array, .init = init);

Test(array, constructor_destructor) {
  Array * a = array(1, 0, 0, NULL, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 0));
  cr_assert(eq(sz, array_capacity(a), 0));
  cr_assert(eq(sz, array_dataSize(a), 1));

  Array * b = array(1, 10, 0, NULL, &customFactory);

  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 0));
  cr_assert(eq(sz, array_capacity(b), 10));
  cr_assert(eq(sz, array_dataSize(b), 1));

  Array * c = array(1, 10, 5, NULL, &customFactory);

  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(sz, array_size(c), 5));
  cr_assert(eq(sz, array_capacity(c), 10));
  cr_assert(eq(sz, array_dataSize(c), 1));
  cr_assert(not(memcmp(array_cData(c), "\0\0\0\0\0", 5)));

  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, clone) {
  int data[] = {1, 2, 3, 4, 5};
  Array * a = array(sizeof(int), 10, 5, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(not(memcmp(array_cData(a), data, 5 * sizeof(int))));

  Array * b = array_clone(a);
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 10));
  cr_assert(eq(sz, array_dataSize(b), sizeof(int)));
  cr_assert(not(memcmp(array_cData(a), array_cData(b), 5 * sizeof(int))));

  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, clear) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));

  array_clear(a);
  cr_assert(eq(sz, array_size(a), 0));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));

  array_destroy(a);
  fini_assert();
}

Test(array, get) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 0), 1));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 1), 2));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 2), 3));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 3), 4));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 4), 5));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -1), 5));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -2), 4));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -3), 3));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -4), 2));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -5), 1));
  cr_assert(eq(ptr, (void *)array_cGet(a, 5), NULL));
  *(int *)array_get(a, 0) = 10;
  cr_assert(eq(i32, *(const int *)array_cGet(a, 0), 10));
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));

  array_destroy(a);
  fini_assert();
}

Test(array, set) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 0), 1));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 1), 2));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 2), 3));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 3), 4));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 4), 5));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -1), 5));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -2), 4));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -3), 3));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -4), 2));
  cr_assert(eq(i32, *(const int *)array_cGet(a, -5), 1));
  cr_assert(eq(i32, array_set(a, 0, (int[]){10}), ARRAY_SUCCESS));
  cr_assert(eq(i32, array_set(a, -2, (int[]){20}), ARRAY_SUCCESS));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 0), 10));
  cr_assert(eq(i32, *(const int *)array_cGet(a, 3), 20));
  cr_assert(eq(i32, array_set(a, 5, (int[]){30}), ARRAY_FAILURE));
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));
  array_destroy(a);
  fini_assert();
}

Test(array, front) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, *(const int *)array_cFront(a), 1));
  cr_assert(eq(i32, *(const int *)array_front(a), 1));
  *(int *)array_front(a) = 10;
  cr_assert(eq(i32, *(const int *)array_cFront(a), 10));
  cr_assert(eq(i32, *(const int *)array_front(a), 10));

  Array * b = array(sizeof(int), 10, 0, NULL, &customFactory);

  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 0));
  cr_assert(eq(sz, array_capacity(b), 10));
  cr_assert(eq(sz, array_dataSize(b), sizeof(int)));
  cr_assert(eq(ptr, (void *)array_cFront(b), NULL));
  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, back) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, *(const int *)array_cBack(a), 5));
  cr_assert(eq(i32, *(const int *)array_back(a), 5));
  *(int *)array_back(a) = 10;
  cr_assert(eq(i32, *(const int *)array_cBack(a), 10));
  cr_assert(eq(i32, *(const int *)array_back(a), 10));

  Array * b = array(sizeof(int), 10, 0, NULL, &customFactory);

  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 0));
  cr_assert(eq(sz, array_capacity(b), 10));
  cr_assert(eq(sz, array_dataSize(b), sizeof(int)));
  cr_assert(eq(ptr, (void *)array_cBack(b), NULL));
  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, data) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(not(memcmp(array_cData(a), data, 5 * sizeof(int))));
  cr_assert(not(memcmp(array_data(a), data, 5 * sizeof(int))));
  array_destroy(a);
  fini_assert();
}

Test(array, reserve) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, array_reserve(a, 5), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(i32, array_reserve(a, 10), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(i32, array_reserve(a, 15), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_capacity(a), 15));
  cr_assert(eq(i32, array_reserve(a, 0), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_capacity(a), 15));
  array_destroy(a);
  fini_assert();
}

Test(array, resize) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, array_resize(a, 5), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(i32, array_resize(a, 10), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 10));
  cr_assert(eq(i32, array_resize(a, 15), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 15));
  cr_assert(eq(i32, array_resize(a, 0), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 0));
  array_destroy(a);
  fini_assert();
}

Test(array, shrink) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 10, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, array_shrink(a), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(i32, array_reserve(a, 10), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(i32, array_shrink(a), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  array_destroy(a);
  fini_assert();
}

Test(array, assign) {
  char data[] = "hello";
  char data2[] = "salut les amis";

  Array * a = array(sizeof(char), 0, 5, data, &customFactory);
  Array * b = array(sizeof(char), 0, 14, data2, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_dataSize(a), sizeof(char)));
  cr_assert(eq(sz, array_size(b), 14));
  cr_assert(eq(sz, array_capacity(b), 14));
  cr_assert(eq(sz, array_dataSize(b), sizeof(char)));
  cr_assert(not(memcmp(array_cData(a), data, 5 * sizeof(char))));
  cr_assert(not(memcmp(array_cData(b), data2, 5 * sizeof(char))));
  cr_assert(eq(i32, array_assign(a, b), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 14));
  cr_assert(ge(sz, array_capacity(a), 14));
  cr_assert(eq(sz, array_dataSize(a), sizeof(char)));
  cr_assert(not(memcmp(array_cData(a), data2, 14 * sizeof(char))));
  cr_assert(eq(i32, array_assignData(b, data, 5), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 14));
  cr_assert(eq(sz, array_dataSize(b), sizeof(char)));
  cr_assert(not(memcmp(array_cData(b), data, 5 * sizeof(char))));
  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, emplace) {
  Array * a = array(sizeof(int), 5, 0, NULL, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 0));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_dataSize(a), sizeof(int)));
  cr_assert(eq(i32, array_emplace(a, 0, 5), ARRAY_SUCCESS));
  cr_assert(eq(i32, array_set(a, 3, (int[]){42}), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(ge(sz, array_capacity(a), 5));
  cr_assert(eq(i32, array_emplace(a, 3, 4), ARRAY_SUCCESS));
  cr_assert(ge(sz, array_capacity(a), 9));
  cr_assert(eq(sz, array_size(a), 9));
  cr_assert(not(memcmp(
      array_cData(a), (int[]){0, 0, 0, 0, 0, 0, 0, 42, 0}, 9 * sizeof(int)
  )));
  cr_assert(eq(i32, array_emplace(a, 10, 5), ARRAY_FAILURE));
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));
  array_destroy(a);
  fini_assert();
}

Test(array, push) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));

  cr_assert(eq(i32, array_push(a, 2, (int[]){42, 42}, 2), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 7));
  cr_assert(ge(sz, array_capacity(a), 7));
  cr_assert(eq(i32, array_push(a, 0, (int[]){42, 42}, 2), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 9));
  cr_assert(ge(sz, array_capacity(a), 9));
  cr_assert(eq(i32, array_push(a, 1, (int[]){1, 2, 3}, 3), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 12));
  cr_assert(ge(sz, array_capacity(a), 12));
  cr_assert(eq(i32, array_push(a, -1, (int[]){1, 2, 3}, 3), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 15));
  cr_assert(ge(sz, array_capacity(a), 15));
  cr_assert(not(memcmp(
      array_cData(a),
      (int[]){42, 1, 2, 3, 42, 1, 2, 42, 42, 3, 4, 1, 2, 3, 5},
      15 * sizeof(int)
  )));
  cr_assert(eq(i32, array_push(a, 16, (int[]){1, 2, 3}, 3), ARRAY_FAILURE));
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));
  cr_assert(eq(i32, array_push(a, -16, (int[]){1, 2, 3}, 3), ARRAY_FAILURE));
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));
  array_destroy(a);
  fini_assert();
}

Test(array, pop) {
  int data[] = {1, 2, 3, 4, 5, 6, 7};
  size_t n = 1, m = 2, o = 5;

  Array * a = array(sizeof(int), 0, 7, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 7));
  cr_assert(eq(sz, array_capacity(a), 7));

  cr_assert(eq(i32, array_pop(a, 1, n, &n), ARRAY_SUCCESS));
  cr_assert(eq(i32, n, 1));
  cr_assert(eq(i32, array_pop(a, 2, m, &m), ARRAY_SUCCESS));
  cr_assert(eq(i32, m, 2));
  cr_assert(eq(i32, array_pop(a, 0, o, &o), ARRAY_SUCCESS));
  cr_assert(eq(i32, o, 4));
  cr_assert(array_empty(a));
  cr_assert(eq(i32, array_pop(a, -13931, o, &o), ARRAY_FAILURE));
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));
  array_destroy(a);
  fini_assert();
}

static int arrayIterFunc(Array * arr, size_t i, void * value, void * param) {
  (void)arr, (void)i, (void)param;
  if (*(int *)value == 42)
    return 1;
  ++*(int *)value;
  return 0;
}

Test(array, forEach) {
  int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  Array * a = array(sizeof(int), 10, 10, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 10));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(i32, array_forEach(a, arrayIterFunc, NULL), ARRAY_SUCCESS));
  cr_assert(not(memcmp(
      array_cData(a), (int[]){2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 10 * sizeof(int)
  )));
  cr_assert(eq(i32, array_set(a, 5, (int[]){42}), ARRAY_SUCCESS));
  cr_assert(eq(i32, array_forEach(a, arrayIterFunc, NULL), ARRAY_FAILURE));
  cr_assert(
      eq(str,
         (char *)array_strerror(),
         "Array iter function returned a non-zero value")
  );
  cr_assert(eq(i32, array_copy(a, data, 10), ARRAY_SUCCESS));
  cr_assert(
      eq(i32, array_forEachWithin(a, 2, 8, arrayIterFunc, NULL), ARRAY_SUCCESS)
  );
  cr_assert(not(memcmp(
      array_cData(a), (int[]){1, 2, 4, 5, 6, 7, 8, 9, 9, 10}, 10 * sizeof(int)
  )));
  array_destroy(a);
  fini_assert();
}

static int arrayCmapFunc(
    const Array * arr, size_t i, void * dst, const void * src, void * param
) {
  (void)arr, (void)i, (void)param;
  *(int *)dst = *(const int *)src + 1;
  ++defaultConstructedElements;
  return 0;
}

Test(array, map) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));

  Array * b = array_cMap(a, sizeof(int), &customFactory, arrayCmapFunc, NULL);
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 5));
  cr_assert(not(memcmp(array_cData(b), (int[]){2, 3, 4, 5, 6}, 5 * sizeof(int)))
  );

  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

static int arrayCreduceFunc(
    const Array * arr, size_t i, void * dst, const void * src, void * param
) {
  (void)arr, (void)i, (void)param;
  *(int *)dst += *(const int *)src;
  return 0;
}

Test(array, reduce) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));

  int * sum = array_cReduce(a, sizeof(int), NULL, arrayCreduceFunc, NULL);
  cr_assert(ne(ptr, sum, NULL));
  cr_assert(eq(i32, *sum, 15));
  free(sum);
  array_destroy(a);
  fini_assert();
}

static int arrayCompareFunc(const void * lhs, const void * rhs, void * param) {
  (void)param;
  const int * l = lhs;
  const int * r = rhs;

  if (*l < *r)
    return -1;
  if (*l > *r)
    return 1;
  return 0;
}

Test(array, sort) {
  int data[] = {10, 1, 9, 2, 8, 3, 7, 4, 6, 5};
  Array * a = array(sizeof(int), 10, 10, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 10));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(eq(i32, array_sort(a, arrayCompareFunc, NULL), ARRAY_SUCCESS));

  cr_assert(not(memcmp(
      array_cData(a), (int[]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 10 * sizeof(int)
  )));
  cr_assert(eq(i32, array_copy(a, data, 10), ARRAY_SUCCESS));
  cr_assert(
      eq(i32, array_sortWithin(a, 3, 7, arrayCompareFunc, NULL), ARRAY_SUCCESS)
  );
  cr_assert(not(memcmp(
      array_cData(a), (int[]){10, 1, 9, 2, 3, 7, 8, 4, 6, 5}, 10 * sizeof(int)
  )));
  cr_assert(eq(i32, array_copy(a, data, 10), ARRAY_SUCCESS));
  Array * b = array_sorted(a, arrayCompareFunc, NULL);
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 10));
  cr_assert(eq(sz, array_capacity(b), 10));
  cr_assert(not(memcmp(
      array_cData(b), (int[]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 10 * sizeof(int)
  )));
  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, compare) {
  int data[] = {1, 2, 3, 4, 5};
  int data2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int data3[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  Array * b = array(sizeof(int), 10, 10, data2, &customFactory);
  Array * c = array(sizeof(int), 10, 10, data3, &customFactory);
  Array * d = array(
      sizeof(char), 0, 5 * sizeof(int) / sizeof(char), data, &customFactory
  );

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_size(b), 10));
  cr_assert(eq(sz, array_capacity(b), 10));
  cr_assert(eq(sz, array_size(c), 10));
  cr_assert(eq(sz, array_capacity(c), 10));
  cr_assert(not(array_compare(a, a, arrayCompareFunc, NULL)));
  cr_assert(lt(i32, array_compare(a, b, arrayCompareFunc, NULL), 0));
  cr_assert(gt(i32, array_compare(b, a, arrayCompareFunc, NULL), 0));
  cr_assert(lt(i32, array_compare(b, c, arrayCompareFunc, NULL), 0));
  cr_assert(gt(i32, array_compare(c, b, arrayCompareFunc, NULL), 0));
  cr_assert(not(array_compareBytes(a, d)));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  array_destroy(d);
  fini_assert();
}

Test(array, extend_concat) {
  int data[] = {1, 2, 3, 4, 5};
  int data2[] = {6, 7, 8, 9, 10};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  Array * b = array(sizeof(int), 5, 5, data2, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 5));
  cr_assert(eq(i32, array_extend(a, b), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 10));
  cr_assert(eq(sz, array_capacity(a), 10));
  cr_assert(not(memcmp(
      array_cData(a), (int[]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 10 * sizeof(int)
  )));

  Array * c = array_concat(a, b, &customFactory);
  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(sz, array_size(c), 15));
  cr_assert(eq(sz, array_capacity(c), 15));
  cr_assert(not(memcmp(
      array_cData(c),
      (int[]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 6, 7, 8, 9, 10},
      15 * sizeof(int)
  )));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

bool arrayPredFn(
    const Array * arr, size_t i, const void * value, const void * param
) {
  (void)arr, (void)i, (void)param;
  return *(const int *)value <= 10;
}

Test(array, all) {
  int data[] = {1, 1, 3, 4, 5};
  int data2[] = {1, 1, 1, 1, 1};
  int data3[] = {100, 2, 1, 3, 4};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  Array * b = array(sizeof(int), 5, 5, data2, &customFactory);
  Array * c = array(sizeof(int), 5, 5, data3, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 5));
  cr_assert(eq(sz, array_size(c), 5));
  cr_assert(eq(sz, array_capacity(c), 5));
  cr_assert(not(array_all(a, (int[]){1})));
  cr_assert(array_allWithin(a, 0, 2, (int[]){1}));
  cr_assert(array_all(b, (int[]){1}));
  cr_assert(not(array_all(c, (int[]){1})));
  cr_assert(array_allIf(a, arrayPredFn, NULL));
  cr_assert(array_allIf(b, arrayPredFn, NULL));
  cr_assert(not(array_allIf(c, arrayPredFn, NULL)));
  cr_assert(array_allWithinIf(a, 1, -1, arrayPredFn, NULL));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, any) {
  int data[] = {1, 2, 3, 4, 5};
  int data2[] = {1, 1, 1, 11, 1};
  int data3[] = {11, 22, 99, 11, 100};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  Array * b = array(sizeof(int), 5, 5, data2, &customFactory);
  Array * c = array(sizeof(int), 5, 5, data3, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 5));
  cr_assert(eq(sz, array_size(c), 5));
  cr_assert(eq(sz, array_capacity(c), 5));
  cr_assert(array_any(a, (int[]){1}));
  cr_assert(array_any(b, (int[]){1}));
  cr_assert(not(array_anyWithin(a, 1, -1, (int[]){1})));
  cr_assert(not(array_any(c, (int[]){1})));
  cr_assert(array_anyIf(a, arrayPredFn, NULL));
  cr_assert(array_anyIf(b, arrayPredFn, NULL));
  cr_assert(not(array_anyIf(c, arrayPredFn, NULL)));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, none) {
  int data[] = {1, 2, 3, 4, 5};
  int data2[] = {1, 1, 1, 11, 1};
  int data3[] = {11, 22, 99, 11, 100};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  Array * b = array(sizeof(int), 5, 5, data2, &customFactory);
  Array * c = array(sizeof(int), 5, 5, data3, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 5));
  cr_assert(eq(sz, array_size(c), 5));
  cr_assert(eq(sz, array_capacity(c), 5));
  cr_assert(not(array_none(a, (int[]){1})));
  cr_assert(not(array_none(b, (int[]){1})));
  cr_assert(array_noneWithin(a, 1, -1, (int[]){1}));
  cr_assert(array_none(c, (int[]){1}));
  cr_assert(not(array_noneIf(a, arrayPredFn, NULL)));
  cr_assert(not(array_noneIf(b, arrayPredFn, NULL)));
  cr_assert(array_noneIf(c, arrayPredFn, NULL));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, fill) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(i32, array_fill(a, (int[]){42}), ARRAY_SUCCESS));
  cr_assert(
      not(memcmp(array_cData(a), (int[]){42, 42, 42, 42, 42}, 5 * sizeof(int)))
  );
  cr_assert(eq(i32, array_fillWithin(a, 1, 4, (int[]){0}), ARRAY_SUCCESS));
  cr_assert(
      not(memcmp(array_cData(a), (int[]){42, 0, 0, 0, 42}, 5 * sizeof(int)))
  );
  cr_assert(eq(i32, array_fillWithin(a, 0, 0, (int[]){0}), ARRAY_SUCCESS));
  cr_assert(
      not(memcmp(array_cData(a), (int[]){42, 0, 0, 0, 42}, 5 * sizeof(int)))
  );
  cr_assert(
      eq(i32, array_fillWithin(a, 12013, 101313, (int[]){42}), ARRAY_FAILURE)
  );
  cr_assert(eq(str, (char *)array_strerror(), "Index out of range"));
  array_destroy(a);
  fini_assert();
}

Test(array, copy) {
  int data[] = {1, 2, 3, 4, 5};
  int data2[] = {6, 7, 8, 9, 10};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);
  Array * b = array(sizeof(int), 5, 5, data2, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  cr_assert(eq(sz, array_size(b), 5));
  cr_assert(eq(sz, array_capacity(b), 5));
  cr_assert(eq(i32, array_copy(a, data2, 5), ARRAY_SUCCESS));
  cr_assert(not(array_compareBytes(a, b)));
  cr_assert(eq(i32, array_copy(b, data, 5), ARRAY_SUCCESS));
  cr_assert(not(array_compareData(b, data, 5, arrayCompareFunc, NULL)));
  cr_assert(eq(i32, array_copyWithin(b, 1, 3, data2, 5), ARRAY_SUCCESS));
  cr_assert(not(
      array_compareData(b, (int[]){1, 6, 7, 4, 5}, 5, arrayCompareFunc, NULL)
  ));
  cr_assert(eq(i32, array_copyWithin(b, 0, 0, data, 2), ARRAY_SUCCESS));
  cr_assert(not(
      array_compareData(b, (int[]){1, 6, 7, 4, 5}, 5, arrayCompareFunc, NULL)
  ));
  cr_assert(eq(i32, array_copyWithin(a, 0, 1, data, 5), ARRAY_SUCCESS));
  cr_assert(not(
      array_compareData(a, (int[]){1, 7, 8, 9, 10}, 5, arrayCompareFunc, NULL)
  ));
  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, filter) {
  int data[] = {99, 2, 1, 2903, 2, 4};

  Array * a = array(sizeof(int), 6, 6, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 6));
  cr_assert(eq(sz, array_capacity(a), 6));

  Array * b = array_filtered(a, (int[]){2});

  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(sz, array_size(b), 4));
  array_filter(a, (int[]){2});
  cr_assert(eq(sz, array_size(a), 4));
  cr_assert(not(array_compareBytes(a, b)));
  cr_assert(not(memcmp(array_cData(a), (int[]){99, 1, 2903, 4}, 4 * sizeof(int))
  ));
  cr_assert(eq(i32, array_assignData(a, data, 6), ARRAY_SUCCESS));
  array_filterIf(a, arrayPredFn, NULL);
  cr_assert(eq(sz, array_size(a), 4));
  cr_assert(not(memcmp(array_cData(a), (int[]){2, 1, 2, 4}, 4 * sizeof(int))));
  cr_assert(eq(i32, array_assignData(a, data, 6), ARRAY_SUCCESS));
  cr_assert(eq(i32, array_filterWithin(a, 0, -2, (int[]){2}), ARRAY_SUCCESS));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(
      not(memcmp(array_cData(a), (int[]){99, 1, 2903, 2, 4}, 5 * sizeof(int)))
  );
  array_destroy(a);
  array_destroy(b);
  fini_assert();
}

Test(array, replace) {
  int data[] = {1, 2, 3, 2, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));
  Array * b = array_replaced(a, (int[]){2}, (int[]){42});
  cr_assert(ne(ptr, b, NULL));
  cr_assert(eq(i32, array_replace(a, (int[]){2}, (int[]){42}), ARRAY_SUCCESS));
  cr_assert(not(array_compareBytes(a, b)));
  cr_assert(
      not(memcmp(array_cData(a), (int[]){1, 42, 3, 42, 5}, 5 * sizeof(int)))
  );
  Array * c = array_replacedIf(a, arrayPredFn, NULL, (int[]){0});
  cr_assert(ne(ptr, c, NULL));
  cr_assert(
      eq(i32, array_replaceIf(a, arrayPredFn, NULL, (int[]){0}), ARRAY_SUCCESS)
  );
  cr_assert(not(array_compareBytes(a, c)));
  cr_assert(
      not(memcmp(array_cData(a), (int[]){0, 42, 0, 42, 0}, 5 * sizeof(int)))
  );
  cr_assert(eq(
      i32, array_replaceWithin(a, 1, 3, (int[]){42}, (int[]){1}), ARRAY_SUCCESS
  ));
  cr_assert(not(memcmp(array_cData(a), (int[]){0, 1, 0, 42, 0}, 5 * sizeof(int))
  ));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, reverse) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));

  Array * b = array_reversed(a);

  cr_assert(ne(ptr, b, NULL));
  array_reverse(a);
  cr_assert(not(memcmp(array_cData(a), (int[]){5, 4, 3, 2, 1}, 5 * sizeof(int)))
  );
  cr_assert(not(array_compareBytes(a, b)));

  Array * c = array_reversedWithin(a, 1, 4);

  cr_assert(ne(ptr, c, NULL));
  cr_assert(eq(i32, array_reverseWithin(a, 1, 4), ARRAY_SUCCESS));
  cr_assert(not(memcmp(array_cData(a), (int[]){5, 2, 3, 4, 1}, 5 * sizeof(int)))
  );
  cr_assert(not(array_compareBytes(a, c)));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, find_index) {
  int data[] = {100, 99, 98, 1, 2, 3, 97, 96, 95};

  Array * a = array(sizeof(int), 0, 9, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 9));
  cr_assert(eq(sz, array_capacity(a), 9));

  int * ptr = array_find(a, (int[]){2});
  cr_assert(ne(ptr, ptr, NULL));
  cr_assert(eq(i32, *ptr, 2));

  int * ptr2 = array_findIf(a, arrayPredFn, NULL);

  cr_assert(ne(ptr, ptr2, NULL));
  cr_assert(eq(i32, *ptr2, 1));

  int * ptr3 = array_find(a, (int[]){42});
  cr_assert(eq(ptr, ptr3, NULL));

  int * ptr4 = array_rFindIf(a, arrayPredFn, NULL);
  cr_assert(ne(ptr, ptr4, NULL));
  cr_assert(eq(i32, *ptr4, 3));

  ptrdiff_t idx = array_index(a, (int[]){2});
  cr_assert(eq(i32, idx, 4));

  ptrdiff_t idx2 = array_indexIf(a, arrayPredFn, NULL);
  cr_assert(eq(i32, idx2, 3));

  ptrdiff_t idx3 = array_index(a, (int[]){42});
  cr_assert(eq(i32, idx3, -1));

  ptrdiff_t idx4 = array_rIndexIf(a, arrayPredFn, NULL);
  cr_assert(eq(i32, idx4, 5));

  array_destroy(a);
  fini_assert();
}

Test(array, rotate) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));

  Array * b = array_rotated(a, 2);

  cr_assert(ne(ptr, b, NULL));
  array_rotate(a, 2);

  cr_assert(not(memcmp(array_cData(a), (int[]){3, 4, 5, 1, 2}, 5 * sizeof(int)))
  );
  cr_assert(not(array_compareBytes(a, b)));

  Array * c = array_rotatedWithin(a, 2, 5, -1);
  cr_assert(ne(ptr, c, NULL));
  array_rotateWithin(a, 2, 5, -1);

  // for (size_t i = 0; i < 5; ++i) {
  //   printf("%d ", *(int *)array_get(a, i));
  // }
  // printf("\n");
  cr_assert(not(memcmp(array_cData(a), (int[]){3, 4, 2, 5, 1}, 5 * sizeof(int)))
  );
  cr_assert(not(array_compareBytes(a, c)));
  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

Test(array, slice) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));

  Array * b = array_sliced(a, 1, -1);
  cr_assert(ne(ptr, b, NULL));
  array_slice(a, 1, 4);

  cr_assert(not(memcmp(array_cData(a), (int[]){2, 3, 4}, 3 * sizeof(int))));
  cr_assert(not(array_compareBytes(a, b)));

  Array * c = array_sliced(a, 0, 2);
  cr_assert(ne(ptr, c, NULL));
  array_slice(a, 0, 2);
  cr_assert(not(memcmp(array_cData(a), (int[]){2, 3}, 2 * sizeof(int))));
  cr_assert(not(array_compareBytes(a, c)));

  array_destroy(a);
  array_destroy(b);
  array_destroy(c);
  fini_assert();
}

static bool arrayCountPredFn(
    const Array * arr, size_t i, const void * value, const void * param
) {
  (void)arr, (void)i, (void)param;
  return *(const int *)value > 3;
}

Test(array, count) {
  int data[] = {1, 2, 3, 4, 5, 2, 3, 4, 5, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 13, 13, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 13));
  cr_assert(eq(sz, array_capacity(a), 13));

  cr_assert(eq(i32, array_count(a, (int[]){2}), 3));
  cr_assert(eq(i32, array_countIf(a, arrayCountPredFn, NULL), 6));
  cr_assert(eq(i32, array_countWithin(a, 0, -4, (int[]){2}), 2));
  cr_assert(eq(i32, array_countWithinIf(a, 0, -1, arrayCountPredFn, NULL), 5));
  cr_assert(eq(i32, array_countWithin(a, 1, 5, (int[]){2}), 1));
  cr_assert(eq(i32, array_countWithinIf(a, 1, 5, arrayCountPredFn, NULL), 2));
  cr_assert(eq(i32, array_countWithin(a, 6, 12, (int[]){2}), 1));
  cr_assert(eq(i32, array_countWithinIf(a, 6, 12, arrayCountPredFn, NULL), 3));
  cr_assert(eq(i32, array_count(a, (int[]){42}), 0));
  array_destroy(a);
  fini_assert();
}

Test(array, min_max_index) {
  int data[] = {1, 2, 3, 4, 5};

  Array * a = array(sizeof(int), 5, 5, data, &customFactory);

  cr_assert(ne(ptr, a, NULL));
  cr_assert(eq(sz, array_size(a), 5));
  cr_assert(eq(sz, array_capacity(a), 5));

  const int * min = array_cMin(a, arrayCompareFunc, NULL);
  cr_assert(ne(ptr, (void *)min, NULL));
  cr_assert(eq(i32, *min, 1));

  const int * max = array_cMax(a, arrayCompareFunc, NULL);
  cr_assert(ne(ptr, (void *)max, NULL));
  cr_assert(eq(i32, *max, 5));

  ptrdiff_t minIdx = array_minIndex(a, arrayCompareFunc, NULL);
  cr_assert(eq(i32, minIdx, 0));

  ptrdiff_t maxIdx = array_maxIndex(a, arrayCompareFunc, NULL);
  cr_assert(eq(i32, maxIdx, 4));

  array_destroy(a);
  fini_assert();
}
