#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SUCCESS 0
#define ARRAY_FAILURE -1

typedef struct s_array Array;
typedef struct s_array_factory ArrayFactory;

typedef void * ArrayAllocatorFunction(size_t size);
typedef void ArrayDeallocatorFunction(void * ptr);
typedef void * ArrayReallocatorFunction(void * ptr, size_t size);
typedef int ArrayCtorFunction(Array * array, void * data, size_t n);
typedef int
ArrayCopyCtorFunction(Array * array, void * dst, const void * src, size_t n);
typedef void ArrayDtorFunction(Array * array, void * data, size_t n);

typedef int
ArrayIterFunction(Array * array, size_t i, void * value, void * param);
typedef int ArrayCIterFunction(
    const Array * array, size_t i, const void * value, void * param
);
typedef int
ArrayMapFunction(Array * array, size_t i, void * dst, void * src, void * param);
typedef int ArrayCMapFunction(
    const Array * array, size_t i, void * dst, const void * src, void * param
);
typedef int
ArrayCompareFunction(const void * lhs, const void * rhs, void * param);
typedef bool ArrayPredicateFunction(
    const Array * array, size_t i, const void * value, const void * param
);

struct s_array_factory {
  // The allocator function (by default, malloc)
  ArrayAllocatorFunction * allocator;

  // The deallocator function (by default, free)
  ArrayDeallocatorFunction * deallocator;

  // The reallocator function (by default, realloc)
  ArrayReallocatorFunction * reallocator;

  // The constructor function (by default, the memory is zeroed)
  ArrayCtorFunction * constructor;

  // The copy constructor function (by default, the memory is copied byte by
  // byte)
  ArrayCopyCtorFunction * copyConstructor;

  // The destructor function (by default, nothing is done)
  ArrayDtorFunction * destructor;
};

#if defined ARRAY_IMPL || defined ARRAY_USE_IMPL

# include "tools.h"
# include <assert.h>
# include <math.h>
# include <stdlib.h>
# include <string.h>

void array_setDataSize(Array * array, size_t dataSize);
void array_setFactory(Array * array, const ArrayFactory * factory);
void array_setCapacity(Array * array, size_t capacity);
void array_setSize(Array * array, size_t size);
void array_setData(Array * array, uint8_t * data);

size_t array_typeSize(void);

void * array_dataOffset(const Array * array, ptrdiff_t pos);
const void * array_cDataOffset(const Array * array, ptrdiff_t pos);

int array_defaultCtor(Array * array, void * data, size_t n);
int array_defaultCopyCtor(
    Array * array, void * dst, const void * src, size_t n
);
void array_defaultDtor(Array * array, void * data, size_t n);

ArrayFactory array_factory(const ArrayFactory * input);

int array_grow(Array * array, size_t minCapacity);

int array_setupPos(const Array * array, ptrdiff_t * pos);
int array_setupRange(const Array * array, ptrdiff_t * from, ptrdiff_t * to);

# ifdef ARRAY_IMPL

struct s_array {
  size_t dataSize;
  ArrayFactory factory;
  size_t capacity;
  size_t size;
  uint8_t * data;
};

# endif

#endif

enum e_array_error {
  ARR_SUCCESS = 0,
  ARR_ENOMEM = 1,
  ARR_ECTORFAIL = 2,
  ARR_ECPYCTORFAIL = 3,
  ARR_ERANGE = 4,
  ARR_EITERFAIL = 5,
  ARR_EMAPFAIL = 6,
  ARR_EDIFFDATASIZE = 7,
};

#define array_errno *array_errno_location()

uint8_t * array_errno_location(void);

Array * array(
    size_t dataSize,
    size_t capacity,
    size_t size,
    const void * data,
    const ArrayFactory * factory
);
Array * array_clone(const Array * array);
void array_clear(Array * array);
void array_destroy(Array * array);
const char * array_strerror(void);

size_t array_dataSize(const Array * array);
size_t array_size(const Array * array);
bool array_empty(const Array * array);
size_t array_capacity(const Array * array);
const ArrayFactory * array_getFactory(const Array * array);

void * array_get(Array * array, ptrdiff_t pos);
const void * array_cGet(const Array * array, ptrdiff_t pos);
int array_set(Array * array, ptrdiff_t pos, const void * value);
void * array_front(Array * array);
const void * array_cFront(const Array * array);
void * array_back(Array * array);
const void * array_cBack(const Array * array);
void * array_data(Array * array);
const void * array_cData(const Array * array);

int array_reserve(Array * array, size_t capacity);
int array_resize(Array * array, size_t size);
int array_shrink(Array * array);

int array_assign(Array * array, const Array * other);
int array_assignData(Array * array, const void * data, size_t count);

int array_emplace(Array * array, ptrdiff_t pos, size_t count);
int array_emplaceBack(Array * array, size_t count);
int array_emplaceFront(Array * array, size_t count);

int array_push(Array * array, ptrdiff_t pos, const void * values, size_t count);
int array_pushBack(Array * array, const void * values, size_t count);
int array_pushFront(Array * array, const void * values, size_t count);

int array_pop(
    Array * array, ptrdiff_t pos, size_t requestedCount, size_t * count
);
int array_popBack(Array * array, size_t requestedCount, size_t * count);
int array_popFront(Array * array, size_t requestedCount, size_t * count);

int array_forEach(Array * array, ArrayIterFunction * iterFn, void * param);
int array_cForEach(
    const Array * array, ArrayCIterFunction * iterFn, void * param
);
int array_rForEach(Array * array, ArrayIterFunction * iterFn, void * param);
int array_crForEach(
    const Array * array, ArrayCIterFunction * iterFn, void * param
);
int array_forEachWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayIterFunction * iterFn,
    void * param
);
int array_cForEachWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCIterFunction * iterFn,
    void * param
);
int array_rForEachWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayIterFunction * iterFn,
    void * param
);
int array_crForEachWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCIterFunction * iterFn,
    void * param
);

Array * array_map(
    Array * array,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayMapFunction * mapFn,
    void * param
);
Array * array_cMap(
    const Array * array,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayCMapFunction * cmapFn,
    void * param
);
Array * array_rMap(
    Array * array,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayMapFunction * mapFn,
    void * param
);
Array * array_crMap(
    const Array * array,
    size_t dataSize,
    const ArrayFactory * factory,
    ArrayCMapFunction * cmapFn,
    void * param
);

void * array_reduce(
    Array * array,
    size_t dataSize,
    const void * init,
    ArrayMapFunction * mapFn,
    void * param
);
void * array_cReduce(
    const Array * array,
    size_t dataSize,
    const void * init,
    ArrayCMapFunction * cmapFn,
    void * param
);
void * array_rReduce(
    Array * array,
    size_t dataSize,
    const void * init,
    ArrayMapFunction * mapFn,
    void * param
);
void * array_crReduce(
    const Array * array,
    size_t dataSize,
    const void * init,
    ArrayCMapFunction * cmapFn,
    void * param
);

int array_sort(Array * array, ArrayCompareFunction * cmpFn, void * param);
int array_sortWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);
Array *
array_sorted(const Array * array, ArrayCompareFunction * cmpFn, void * param);
Array * array_sortedWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);

int array_compare(
    const Array * lhs,
    const Array * rhs,
    ArrayCompareFunction * cmpFn,
    void * param
);
int array_compareBytes(const Array * lhs, const Array * rhs);
int array_compareData(
    const Array * lhs,
    const void * rhs,
    size_t size,
    ArrayCompareFunction * cmpFn,
    void * param
);

int array_extend(Array * array, const Array * other);
Array *
array_concat(const Array * lhs, const Array * rhs, const ArrayFactory * factory);

bool array_all(const Array * array, const void * value);
bool array_allIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
int array_allWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
int array_allWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

bool array_any(const Array * array, const void * value);
bool array_anyIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
int array_anyWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
int array_anyWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

bool array_none(const Array * array, const void * value);
bool array_noneIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
int array_noneWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
int array_noneWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

int array_fill(Array * array, const void * value);
int array_fillWithin(
    Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);

int array_copy(Array * array, const void * values, size_t count);
int array_copyWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    const void * values,
    size_t count
);

void array_filter(Array * array, const void * value);
void array_filterIf(
    Array * array, ArrayPredicateFunction * predFn, void * param
);
int array_filterWithin(
    Array * array, ptrdiff_t from, ptrdiff_t t, const void * value
);
int array_filterWithinIf(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

Array * array_filtered(const Array * array, const void * value);
Array * array_filteredIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
Array * array_filteredWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
Array * array_filteredWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

int array_replace(Array * array, const void * oldValue, const void * newValue);
int array_replaceIf(
    Array * array,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
);
int array_replaceWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    const void * oldValue,
    const void * newValue
);
int array_replaceWithinIf(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
);

Array * array_replaced(
    const Array * array, const void * oldValue, const void * newValue
);
Array * array_replacedIf(
    const Array * array,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
);
Array * array_replacedWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    const void * oldValue,
    const void * newValue
);
Array * array_replacedWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param,
    const void * newValue
);

void array_reverse(Array * array);
int array_reverseWithin(Array * array, ptrdiff_t from, ptrdiff_t to);

Array * array_reversed(const Array * array);
Array * array_reversedWithin(const Array * array, ptrdiff_t from, ptrdiff_t to);

void * array_find(Array * array, const void * value);
const void * array_cFind(const Array * array, const void * value);
void * array_rFind(Array * array, const void * value);
const void * array_crFind(const Array * array, const void * value);
void *
array_findIf(Array * array, ArrayPredicateFunction * predFn, void * param);
const void * array_cFindIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
void *
array_rFindIf(Array * array, ArrayPredicateFunction * predFn, void * param);
const void * array_crFindIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
void * array_findWithin(
    Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
const void * array_cFindWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
void * array_rFindWithin(
    Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
const void * array_crFindWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
void * array_findWithinIf(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);
const void * array_cFindWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

void * array_rFindWithinIf(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);
const void * array_crFindWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

ptrdiff_t array_index(const Array * array, const void * value);
ptrdiff_t array_rIndex(const Array * array, const void * value);
ptrdiff_t array_indexIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
ptrdiff_t array_rIndexIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
ptrdiff_t array_indexWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
ptrdiff_t array_rIndexWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
ptrdiff_t array_indexWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);
ptrdiff_t array_rIndexWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

int array_rotate(Array * array, ptrdiff_t n);
int array_rotateWithin(
    Array * array, ptrdiff_t from, ptrdiff_t to, ptrdiff_t n
);
Array * array_rotated(const Array * array, ptrdiff_t n);
Array * array_rotatedWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, ptrdiff_t n
);

int array_slice(Array * array, ptrdiff_t from, ptrdiff_t to);
Array * array_sliced(const Array * array, ptrdiff_t from, ptrdiff_t to);

int array_count(const Array * array, const void * value);
int array_countIf(
    const Array * array, ArrayPredicateFunction * predFn, void * param
);
int array_countWithin(
    const Array * array, ptrdiff_t from, ptrdiff_t to, const void * value
);
int array_countWithinIf(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayPredicateFunction * predFn,
    void * param
);

void * array_min(Array * array, ArrayCompareFunction * cmpFn, void * param);
const void *
array_cMin(const Array * array, ArrayCompareFunction * cmpFn, void * param);
void * array_minWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);
const void * array_cMinWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);

ptrdiff_t
array_minIndex(const Array * array, ArrayCompareFunction * cmpFn, void * param);
ptrdiff_t array_minIndexWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);

void * array_max(Array * array, ArrayCompareFunction * cmpFn, void * param);
const void *
array_cMax(const Array * array, ArrayCompareFunction * cmpFn, void * param);
void * array_maxWithin(
    Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);
const void * array_cMaxWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);

ptrdiff_t
array_maxIndex(const Array * array, ArrayCompareFunction * cmpFn, void * param);
ptrdiff_t array_maxIndexWithin(
    const Array * array,
    ptrdiff_t from,
    ptrdiff_t to,
    ArrayCompareFunction * cmpFn,
    void * param
);

#endif
