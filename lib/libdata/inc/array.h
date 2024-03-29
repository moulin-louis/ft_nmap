/**
 * @brief This variable represents an array of data.
 *
 * It stores a collection of data elements of a specific type.
 * The size of the array is determined by the number of elements it contains.
 * The elements can be accessed using their index position.
 */
#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SUCCESS 0
#define ARRAY_FAILURE -1

typedef struct s_array Array;
typedef struct s_array_factory ArrayFactory;

/*
** The function signature for the allocator function used by `ArrayFactory`.
**
** @param size The size of the memory to allocate.
**
** @return A pointer to the allocated memory, or `NULL` in case of failure.
*/
typedef void* ArrayAllocatorFunction(size_t size);

/*
** The function signature for the deallocator function used by `ArrayFactory`.
**
** @param ptr The pointer to the memory to deallocate.
*/
typedef void ArrayDeallocatorFunction(void* ptr);

/*
** The function signature for the reallocator function used by `ArrayFactory`.
**
** @param ptr The pointer to the memory to reallocate.
** @param size The new size of the memory.
**
** @return A pointer to the reallocated memory, or `NULL` in case of failure.
*/
typedef void* ArrayReallocatorFunction(void* ptr, size_t size);

/*
** The function signature for the default constructor function used by
** `ArrayFactory`.
**
** @param array The array to construct.
** @param data A pointer to the data to initialize inside the array.
** @param n The number of elements to initialize.
**
** @return `0` in case of success, any other value otherwise.
*/
typedef int ArrayCtorFunction(Array* array, void* data, size_t n);

/*
** The function signature for the copy constructor function used by
** `ArrayFactory`.
**
** @param array The array to construct.
** @param dst A pointer to the destination memory inside the array.
** @param src A pointer to the source memory.
** @param n The number of elements to copy.
**
** @return `0` in case of success, any other value otherwise.
*/
typedef int ArrayCopyCtorFunction(Array* array, void* dst, const void* src, size_t n);

/*
** The function signature for the destructor function used by `ArrayFactory`.
**
** @param array The array to destroy.
** @param data A pointer to the data to destroy inside the array.
** @param n The number of elements to destroy.
*/
typedef void ArrayDtorFunction(Array* array, void* data, size_t n);

/*
** The function signature for the iterator function used by the `array_forEach`
** family of functions.
**
** @param array The array.
** @param i The index of the element.
** @param value A pointer to the current element.
** @param param An additional parameter passed to the function (can be `NULL`).
**
** @return `0` in case of success, any other value otherwise.
*/
typedef int ArrayIterFunction(Array* array, size_t i, void* value, void* param);

/*
** The function signature for the const iterator function used by the
** `array_cForEach` family of functions.
**
** @param array The array.
** @param i The index of the element.
** @param value A pointer to the current element.
** @param param An additional parameter passed to the function (can be `NULL`).
**
** @return `0` in case of success, any other value otherwise.
**
** @note This is the const version of `ArrayIterFunction`.
*/
typedef int ArrayCIterFunction(const Array* array, size_t i, const void* value, void* param);

/*
** The function signature for the map function used by the `array_map` and
** `array_reduce` family of functions.
**
** @param array The array.
** @param i The index of the element.
** @param dst A pointer to the destination memory inside the array.
** @param src A pointer to the source memory.
** @param param An additional parameter passed to the function (can be `NULL`).
**
** @return `0` in case of success, any other value otherwise.
*/
typedef int ArrayMapFunction(Array* array, size_t i, void* dst, void* src, void* param);

/*
** The function signature for the const map function used by the `array_cMap`
** and `array_cReduce` family of functions.
**
** @param array The array.
** @param i The index of the element.
** @param dst A pointer to the destination memory inside the array.
** @param src A pointer to the source memory.
** @param param An additional parameter passed to the function (can be `NULL`).
**
** @return `0` in case of success, any other value otherwise.
**
** @note This is the const version of `ArrayMapFunction`.
*/
typedef int ArrayCMapFunction(const Array* array, size_t i, void* dst, const void* src, void* param);

/*
** The function signature for the compare function used by the `array_sort`,
** `array_compare`, `array_min` and `array_max` family of functions.
**
** @param lhs A pointer to the left-hand side element.
** @param rhs A pointer to the right-hand side element.
** @param param An additional parameter passed to the function (can be `NULL`).
**
** @return A negative value if `lhs` is less than `rhs`, a positive value if
**         `lhs` is greater than `rhs`, or `0` if they are equal.
*/
typedef int ArrayCompareFunction(const void* lhs, const void* rhs, void* param);

/*
** The function signature for the predicate function used by the `array_XIf`
** family of functions.
**
** @param array The array.
** @param i The index of the element.
** @param value A pointer to the current element.
** @param param An additional parameter passed to the function (can be `NULL`).
**
** @return `true` if the predicate is satisfied, `false` otherwise.
*/
typedef bool ArrayPredicateFunction(const Array* array, size_t i, const void* value, const void* param);

struct s_array_factory {
  // The allocator function (by default, `malloc`)
  ArrayAllocatorFunction* allocator;

  // The deallocator function (by default, `free`)
  ArrayDeallocatorFunction* deallocator;

  // The reallocator function (by default, `realloc`)
  ArrayReallocatorFunction* reallocator;

  // The constructor function (by default, the memory is zeroed using `memset`)
  ArrayCtorFunction* constructor;

  // The copy constructor function (by default, the memory is copied byte by
  // byte using `memcpy`)
  ArrayCopyCtorFunction* copyConstructor;

  // The destructor function (by default, nothing is done)
  ArrayDtorFunction* destructor;
};

#if defined ARRAY_IMPL || defined ARRAY_USE_IMPL

# include <assert.h>
# include <math.h>
# include <stdatomic.h>
# include <stdint.h>
# include <stdlib.h>
# include <string.h>
# include "tools.h"

void array_setDataSize(Array* array, size_t dataSize);
void array_setFactory(Array* array, const ArrayFactory* factory);
void array_setCapacity(Array* array, size_t capacity);
void array_setSize(Array* array, size_t size);
void array_setData(Array* array, uint8_t* data);

size_t array_typeSize(void);

void* array_dataOffset(const Array* array, ptrdiff_t pos);
const void* array_cDataOffset(const Array* array, ptrdiff_t pos);

int array_defaultCtor(Array* array, void* data, size_t n);
int array_defaultCopyCtor(Array* array, void* dst, const void* src, size_t n);
void array_defaultDtor(Array* array, void* data, size_t n);

ArrayFactory array_factory(const ArrayFactory* input);

int array_grow(Array* array, size_t minCapacity);

int array_setupPos(const Array* array, ptrdiff_t* pos);
int array_setupRange(const Array* array, ptrdiff_t* from, ptrdiff_t* to);

# ifdef ARRAY_IMPL

struct s_array {
  size_t dataSize;
  ArrayFactory factory;
  size_t capacity;
  size_t size;
  uint8_t* data;
};

# endif

#endif

enum e_array_error {
  // Success
  ARR_SUCCESS = 0,

  // Out of memory
  ARR_ENOMEM = 1,

  // Constructor function returned a non-zero value
  ARR_ECTORFAIL = 2,

  // Copy-constructor function returned a non-zero value
  ARR_ECPYCTORFAIL = 3,

  // Index out of range
  ARR_ERANGE = 4,

  // Array iter function returned a non-zero value
  ARR_EITERFAIL = 5,

  // Array map function returned a non-zero value
  ARR_EMAPFAIL = 6,

  // Data sizes of arrays are different
  ARR_EDIFFDATASIZE = 7,
};

#define array_errno *array_errno_location()

_Atomic uint8_t* array_errno_location(void);

/*
** Create a new array.
**
** @param dataSize The size (in bytes) of a single element.
** @param capacity The initial capacity of the array.
** @param size The initial size of the array.
** @param data The initial data of the array, or `NULL` for default
**             initialization.
** @param factory The custom factory to use, or `NULL` to use the default one.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECTORFAIL` if the default constructor failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
**
** @note If `size` is greater than `capacity`, `capacity` will be set to `size`.
** @note If `data` is provided, it must point to a buffer of at least `size *
**       dataSize` bytes.
** @note With the `factory` parameter, you can set a custom allocator,
**       deallocator, reallocator, constructor, copy constructor, and destructor
**       for the array, each field of `factory` that is set to `NULL` will be
**       replaced by the default one. (see `struct s_array_factory` for more
**       details).
*/
Array* array(size_t dataSize, size_t capacity, size_t size, const void* data, const ArrayFactory* factory);

/*
** Clone an array.
**
** @param array The array to clone.
**
** @return The cloned array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
*/
Array* array_clone(const Array* array);

/*
** Clear an array.
**
** @param array The array to clear.
**
** @note The array will be empty after this operation.
*/
void array_clear(Array* array);

/*
** Destroy an array.
**
** @param array The array to destroy.
**
** @note The array will be freed after this operation, and no
**       operation should be done on it after this call.
*/
void array_destroy(Array* array);

/*
** Get the description for the last error that occured.
**
** @return The last error message.
** @note This function is thread-safe, `array_errno` too.
*/
const char* array_strerror(void);

/*
** Get the size of a single element in the array.
**
** @param array The array.
**
** @return The size (in bytes) of a single element in the array.
*/
size_t array_dataSize(const Array* array);

/*
** Get the size of the array.
**
** @param array The array.
**
** @return The size of the array.
*/
size_t array_size(const Array* array);

/*
** Check if the array is empty.
**
** @param array The array.
**
** @return `true` if the array is empty, `false` otherwise.
*/
bool array_empty(const Array* array);

/*
** Get the capacity of the array.
**
** @param array The array.
**
** @return The capacity of the array (the number of elements it can hold without
**         a reallocation).
*/
size_t array_capacity(const Array* array);

/*
** Get the factory of the array.
**
** @param array The array.
**
** @return A const pointer to the factory of the array.
*/
const ArrayFactory* array_getFactory(const Array* array);

/*
** Get the element at the specified position.
**
** @param array The array.
** @param pos The position of the element (if negative, it will expand to
**            `array_size(array) + pos`).
**
** @return A pointer to the element at the specified position, or `NULL` if the
**         position is out of range.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the position is out of range.
*/
void* array_get(Array* array, ptrdiff_t pos);

/*
** Get the element at the specified position.
**
** @param array The array.
** @param pos The position of the element (if negative, it will expand to
**            `array_size(array) + pos`).
**
** @return A const pointer to the element at the specified position, or `NULL`
**         if the position is out of range.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the position is out of range.
**
** @note This is the const version of `array_get`.
*/
const void* array_cGet(const Array* array, ptrdiff_t pos);

/*
** Set the element at the specified position.
**
** @param array The array.
** @param pos The position of the element (if negative, it will expand to
**            `array_size(array) + pos`).
** @param value The value to set.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the position is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
*/
int array_set(Array* array, ptrdiff_t pos, const void* value);

/*
** Get the first element of the array.
**
** @param array The array.
**
** @return A pointer to the first element of the array, or `NULL` if the array
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
*/
void* array_front(Array* array);

/*
** Get the first element of the array.
**
** @param array The array.
**
** @return A const pointer to the first element of the array, or `NULL` if the
**         array is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is the const version of `array_front`.
*/
const void* array_cFront(const Array* array);

/*
** Get the last element of the array.
**
** @param array The array.
**
** @return A pointer to the last element of the array, or `NULL` if the array
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
*/
void* array_back(Array* array);

/*
** Get the last element of the array.
**
** @param array The array.
**
** @return A const pointer to the last element of the array, or `NULL` if the
**         array is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is the const version of `array_back`.
*/
const void* array_cBack(const Array* array);

/*
** Get the data of the array.
**
** @param array The array.
**
** @return A pointer to the data of the array.
*/
void* array_data(Array* array);

/*
** Get the data of the array.
**
** @param array The array.
**
** @return A const pointer to the data of the array.
**
** @note This is the const version of `array_data`.
*/
const void* array_cData(const Array* array);

/*
** Reserve memory for the array.
**
** @param array The array.
** @param capacity The new capacity of the array.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note If `capacity` is less than the current capacity, nothing will be done.
*/
int array_reserve(Array* array, size_t capacity);

/*
** Resize the array.
**
** @param array The array.
** @param size The new size of the array.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECTORFAIL` if the default constructor failed.
**
** @note If `size` is greater than the current size, the new elements will be
**       default-constructed.
** @note If `size` is less than the current size, the `array_size(array) - size`
**       last elements will be destroyed.
*/
int array_resize(Array* array, size_t size);

/*
** Shrink the array to fit its size.
**
** @param array The array.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note After this operation, the capacity of the array will be equal to its
**       size.
*/
int array_shrink(Array* array);

/*
** Assign the data of another array to this array.
**
** @param array The array.
** @param other The other array.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
*/
int array_assign(Array* array, const Array* other);

/*
** Assign raw data to the array.
**
** @param array The array.
** @param data The data to assign.
** @param count The number of elements to assign.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `data` must point to a buffer of at least `count *
**        array_dataSize(array)` bytes.
*/
int array_assignData(Array* array, const void* data, size_t count);

/*
** Insert default constructed elements into the array.
**
** @param array The array.
** @param pos The position where to insert the elements (if negative, it will
**            expand to `array_size(array) + pos`).
** @param count The number of elements to insert.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECTORFAIL` if the default constructor failed.
** @throws - `ARR_ERANGE` if the position is out of range.
*/
int array_emplace(Array* array, ptrdiff_t pos, size_t count);

/*
** Insert default constructed elements at the back of the array.
**
** @param array The array.
** @param count The number of elements to insert.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECTORFAIL` if the default constructor failed.
**
** @note This is equivalent to `array_emplace(array, array_size(array), count)`.
*/
int array_emplaceBack(Array* array, size_t count);

/*
** Insert default constructed elements at the front of the array.
**
** @param array The array.
** @param count The number of elements to insert.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECTORFAIL` if the default constructor failed.
**
** @note This is equivalent to `array_emplace(array, 0, count)`.
*/
int array_emplaceFront(Array* array, size_t count);

/*
** Insert elements into the array.
**
** @param array The array.
** @param pos The position where to insert the elements (if negative, it will
**            expand to `array_size(array) + pos`).
** @param values The values to insert.
** @param count The number of elements to insert.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
** @throws - `ARR_ERANGE` if the position is out of range.
**
** @note `values` must point to a buffer of at least `count *
**        array_dataSize(array)` bytes.
*/
int array_push(Array* array, ptrdiff_t pos, const void* values, size_t count);

/*
** Insert elements at the back of the array.
**
** @param array The array.
** @param values The values to insert.
** @param count The number of elements to insert.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note This is equivalent to `array_push(array, array_size(array), values,
**        count)`.
*/
int array_pushBack(Array* array, const void* values, size_t count);

/*
** Insert elements at the front of the array.
**
** @param array The array.
** @param values The values to insert.
** @param count The number of elements to insert.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note This is equivalent to `array_push(array, 0, values, count)`.
*/
int array_pushFront(Array* array, const void* values, size_t count);

/*
** Remove elements from the array.
**
** @param array The array.
** @param pos The position of the first element to remove (if negative, it will
**            expand to `array_size(array) + pos`).
** @param requestedCount The number of elements to remove.
** @param count The number of elements actually removed (can be `NULL`). If
**              `count` is `NULL`, the actual number of elements removed will
**              not be returned.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the position is out of range.
*/
int array_pop(Array* array, ptrdiff_t pos, size_t requestedCount, size_t* count);

/*
** Remove elements from the back of the array.
**
** @param array The array.
** @param requestedCount The number of elements to remove.
** @param count The number of elements actually removed (can be `NULL`). If
**              `count` is `NULL`, the actual number of elements removed will
**              not be returned.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the position is out of range.
**
** @note This is equivalent to `array_pop(array, -requestedCount,
**       requestedCount, count)`.
*/
int array_popBack(Array* array, size_t requestedCount, size_t* count);

/*
** Remove elements from the front of the array.
**
** @param array The array.
** @param requestedCount The number of elements to remove.
** @param count The number of elements actually removed (can be `NULL`). If
**              `count` is `NULL`, the actual number of elements removed will
**              not be returned.
**
** @return `ARRAY_SUCCESS`.
**
** @note This is equivalent to `array_pop(array, 0, requestedCount, count)`.
*/
int array_popFront(Array* array, size_t requestedCount, size_t* count);

/*
** Apply a callback function to each element of the array, starting from the
** beginning.
**
** @param array The array.
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note This is equivalent to `array_forEachWithin(array, 0, array_size(array),
**       iterFn, param)`.
*/
int array_forEach(Array* array, ArrayIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of the array, starting from the
** beginning (the function won't be able to mutate the elements in place).
**
** @param array The array.
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note This is the const version of `array_forEach`.
** @note This is equivalent to `array_cForEachWithin(array, 0,
**       array_size(array), iterFn, param)`.
*/
int array_cForEach(const Array* array, ArrayCIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of the array, starting from the
** end.
**
** @param array The array.
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
*/
int array_rForEach(Array* array, ArrayIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of the array, starting from the
** end (the function won't be able to mutate the elements in place).
**
** @param array The array.
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note This is the const version of `array_rForEach`.
** @note This is equivalent to `array_crForEachWithin(array, 0,
**       array_size(array), iterFn, param)`.
*/
int array_crForEach(const Array* array, ArrayCIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of a specified range within the
** array, starting from the beginning.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note If `from` is greater than `to`, the range will be empty.
*/
int array_forEachWithin(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of a specified range within the
** array, starting from the beginning (the function won't be able to mutate the
** elements in place).
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note This is the const version of `array_forEachWithin`.
** @note If `from` is greater than `to`, the range will be empty.
*/
int array_cForEachWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of a specified range within the
** array, starting from the end.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note If `from` is greater than `to`, the range will be empty.
*/
int array_rForEachWithin(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayIterFunction* iterFn, void* param);

/*
** Apply a callback function to each element of a specified range within the
** array, starting from the end (the function won't be able to mutate the
** elements in place).
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param iterFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_EITERFAIL` if the `iterFn` failed (which means that it
**         returned a non-zero value).
**
** @note This is the const version of `array_rForEachWithin`.
** @note If `from` is greater than `to`, the range will be empty.
*/
int array_crForEachWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCIterFunction* iterFn, void* param);

/*
** Create a new array resulting from successive applications of a callback
** function to each element of the array, starting from the beginning.
**
** @param array The array.
** @param dataSize The size (in bytes) of a single element in the new array.
** @param factory The factory to use for the new array, or `NULL` to use the
**                default one.
** @param mapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note With the `factory` parameter, you can set a custom allocator,
**       deallocator, reallocator, constructor, copy constructor, and destructor
**       for the new array, each field of `factory` that is set to `NULL` will
**       be replaced by the default one. (see `struct s_array_factory` for more
**       details).
*/
Array* array_map(Array* array, size_t dataSize, const ArrayFactory* factory, ArrayMapFunction* mapFn, void* param);

/*
** Create a new array resulting from successive applications of a callback
** function to each element of the array, starting from the beginning (the
** function won't be able to mutate the elements in place).
**
** @param array The array.
** @param dataSize The size (in bytes) of a single element in the new array.
** @param factory The factory to use for the new array, or `NULL` to use the
**                default one.
** @param cmapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note With the `factory` parameter, you can set a custom allocator,
**       deallocator, reallocator, constructor, copy constructor, and destructor
**       for the new array, each field of `factory` that is set to `NULL` will
**       be replaced by the default one. (see `struct s_array_factory` for more
**       details).
** @note This is the const version of `array_map`.
*/
Array* array_cMap(const Array* array, size_t dataSize, const ArrayFactory* factory, ArrayCMapFunction* cmapFn,
                  void* param);

/*
** Create a new array resulting from successive applications of a callback
** function to each element of the array, starting from the end.
**
** @param array The array.
** @param dataSize The size (in bytes) of a single element in the new array.
** @param factory The factory to use for the new array, or `NULL` to use the
**                default one.
** @param mapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note With the `factory` parameter, you can set a custom allocator,
**       deallocator, reallocator, constructor, copy constructor, and destructor
**       for the new array, each field of `factory` that is set to `NULL` will
**       be replaced by the default one. (see `struct s_array_factory` for more
**       details).
** @note The returned array will not be reversed, only the order of the callback
**       function applications will be.
*/
Array* array_rMap(Array* array, size_t dataSize, const ArrayFactory* factory, ArrayMapFunction* mapFn, void* param);

/*
** Create a new array resulting from successive applications of a callback
** function to each element of the array, starting from the end (the function
** won't be able to mutate the elements in place).
**
** @param array The array.
** @param dataSize The size (in bytes) of a single element in the new array.
** @param factory The factory to use for the new array, or `NULL` to use the
**                default one.
** @param cmapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note With the `factory` parameter, you can set a custom allocator,
**       deallocator, reallocator, constructor, copy constructor, and destructor
**       for the new array, each field of `factory` that is set to `NULL` will
**       be replaced by the default one. (see `struct s_array_factory` for more
**       details).
** @note The returned array will not be reversed, only the order of the callback
**       function applications will be.
** @note This is the const version of `array_rMap`.
*/
Array* array_crMap(const Array* array, size_t dataSize, const ArrayFactory* factory, ArrayCMapFunction* cmapFn,
                   void* param);

/*
** Reduce the array to a single value (allocated with malloc), starting from the
** beginning.
**
** @param array The array.
** @param dataSize The size (in bytes) of the returned value.
** @param init The initial value of the accumulator (will be copied with
**             `memcpy` at the beginning).
** @param mapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The reduced value (don't forget to free it with `free`), or `NULL` in
**         case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note The map function will receive the accumulator as the `dst` parameter,
**       and the current element of the array as the `src` parameter.
*/
void* array_reduce(Array* array, size_t dataSize, const void* init, ArrayMapFunction* mapFn, void* param);

/*
** Reduce the array to a single value (allocated with malloc), starting from the
** beginning (the function won't be able to mutate the elements in place).
**
** @param array The array.
** @param dataSize The size (in bytes) of the returned value.
** @param init The initial value of the accumulator (will be copied with
**             `memcpy` at the beginning).
** @param cmapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The reduced value (don't forget to free it with `free`), or `NULL` in
**         case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note The map function will receive the accumulator as the `dst` parameter,
**       and the current element of the array as the `src` parameter.
** @note This is the const version of `array_reduce`.
*/
void* array_cReduce(const Array* array, size_t dataSize, const void* init, ArrayCMapFunction* cmapFn, void* param);

/*
** Reduce the array to a single value (allocated with malloc), starting from the
** end.
**
** @param array The array.
** @param dataSize The size (in bytes) of the returned value.
** @param init The initial value of the accumulator (will be copied with
**             `memcpy` at the beginning).
** @param mapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The reduced value (don't forget to free it with `free`), or `NULL` in
**         case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note The map function will receive the accumulator as the `dst` parameter,
**       and the current element of the array as the `src` parameter.
*/
void* array_rReduce(Array* array, size_t dataSize, const void* init, ArrayMapFunction* mapFn, void* param);

/*
** Reduce the array to a single value (allocated with malloc), starting from the
** end (the function won't be able to mutate the elements in place).
**
** @param array The array.
** @param dataSize The size (in bytes) of the returned value.
** @param init The initial value of the accumulator (will be copied with
**             `memcpy` at the beginning).
** @param cmapFn The callback function to apply.
** @param param An additional parameter to pass to the callback function (can be
**              `NULL`).
**
** @return The reduced value (don't forget to free it with `free`), or `NULL` in
**         case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_EMAPFAIL` if the `mapFn` failed (which means that it returned
**         a non-zero value).
**
** @note The map function will receive the accumulator as the `dst` parameter,
**       and the current element of the array as the `src` parameter.
** @note This is the const version of `array_rReduce`.
*/
void* array_crReduce(const Array* array, size_t dataSize, const void* init, ArrayCMapFunction* cmapFn, void* param);

/*
** Sort an array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note This is equivalent to `array_sortWithin(array, 0, array_size(array),
**       cmpFn, param)`.
** @note The merge sort algorithm is used.
*/
int array_sort(Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Sort a range within an array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note The merge sort algorithm is used.
*/
int array_sortWithin(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn, void* param);

/*
** Create a new array resulting from the sorting of the elements of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note This is equivalent to `array_sortedWithin(array, 0, array_size(array),
**       cmpFn, param)`.
** @note The merge sort algorithm is used.
*/
Array* array_sorted(const Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Create a new array resulting from the sorting of a range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note The merge sort algorithm is used.
*/
Array* array_sortedWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn, void* param);

/*
** Lexicographically compare two arrays.
**
** @param lhs The left-hand side array.
** @param rhs The right-hand side array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A negative value if `lhs` is less than `rhs`, a positive value if
**         `lhs` is greater than `rhs`, or `0` if they are equal.
**
** @note Like `strcmp`, if one of the array is the prefix of the other, the
**       shorter array is considered less than the longer one.
*/
int array_compare(const Array* lhs, const Array* rhs, ArrayCompareFunction* cmpFn, void* param);

/*
** Lexicographically compare two arrays byte by byte using `memcmp`.
**
** @param lhs The left-hand side array.
** @param rhs The right-hand side array.
**
** @return A negative value if `lhs` is less than `rhs`, a positive value if
**         `lhs` is greater than `rhs`, or `0` if they are equal.
**
** @note Like `strcmp`, if one of the array is the prefix of the other, the
**       shorter array is considered less than the longer one.
*/
int array_compareBytes(const Array* lhs, const Array* rhs);

/*
** Lexicographically compare an array with a buffer.
**
** @param lhs The left-hand side array.
** @param rhs The right-hand side buffer.
** @param size The size of the buffer.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A negative value if `lhs` is less than `rhs`, a positive value if
**         `lhs` is greater than `rhs`, or `0` if they are equal.
**
** @note Like `strcmp`, if one of the array is the prefix of the other, the
**       shorter array is considered less than the longer one.
** @note `rhs` must point to a buffer of at least `size * array_dataSize(lhs)`
**       bytes.
*/
int array_compareData(const Array* lhs, const void* rhs, size_t size, ArrayCompareFunction* cmpFn, void* param);

/*
** Append the elements of another array to the end of the array.
**
** @param array The array.
** @param other The array to append.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
** @throws - `ARR_EDIFFDATASIZE` if the data sizes of the two arrays differ.
**
** @note It will use the factory of `array` to copy the elements, so even if
**       the function works with two arrays of different types that have the
**       same data size, it may not work as intended.
*/
int array_extend(Array* array, const Array* other);

/*
** Concatenate two arrays into a new one.
**
** @param lhs The left-hand side array.
** @param rhs The right-hand side array.
** @param factory The factory to use for the new array, or `NULL` to use the
**                default one.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
** @throws - `ARR_EDIFFDATASIZE` if the data sizes of the two arrays differ.
**
** @note With the `factory` parameter, you can set a custom allocator,
**       deallocator, reallocator, constructor, copy constructor, and destructor
**       for the new array, each field of `factory` that is set to `NULL` will
**       be replaced by the default one. (see `struct s_array_factory` for more
**       details).
*/
Array* array_concat(const Array* lhs, const Array* rhs, const ArrayFactory* factory);

/*
** Check if all elements of the array are equal to a value.
**
** @param array The array.
** @param value The value to compare to.
**
** @return `true` if all elements are equal to `value`, `false` otherwise.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_allWithin(array, 0, array_size(array),
**       value)`.
*/
bool array_all(const Array* array, const void* value);

/*
** Check if all elements of the array satisfy a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `true` if all elements satisfy the predicate function, `false`
**         otherwise.
**
** @note This is equivalent to `array_allWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
bool array_allIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Check if all elements of a specified range within the array are equal to a
** value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to compare to.
**
** @return `true` if all elements within the range are equal to `value`, `false`
**         otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
int array_allWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Check if all elements of a specified range within the array satisfy a
** predicate function.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `true` if all elements within the range satisfy the predicate
**         function, `false` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
int array_allWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Check if at least one element of the array is equal to a value.
**
** @param array The array.
** @param value The value to compare to.
**
** @return `true` if at least one element is equal to `value`, `false`
**         otherwise.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_anyWithin(array, 0, array_size(array),
**       value)`.
*/
bool array_any(const Array* array, const void* value);

/*
** Check if at least one element of the array satisfies a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `true` if at least one element satisfies the predicate function,
**         `false` otherwise.
**
** @note This is equivalent to `array_anyWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
bool array_anyIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Check if at least one element of a specified range within the array is equal
** to a value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to compare to.
**
** @return `true` if at least one element within the range is equal to `value`,
**         `false` if not, or `ARRAY_FAILURE` in case of error.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
int array_anyWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Check if at least one element of a specified range within the array satisfies
** a predicate function.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `true` if at least one element within the range satisfies the
**         predicate function, `false` if not, or `ARRAY_FAILURE` in case of
**         error.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
int array_anyWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Check if no element of the array is equal to a value.
**
** @param array The array.
** @param value The value to compare to.
**
** @return `true` if no element is equal to `value`, `false` otherwise.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_noneWithin(array, 0, array_size(array),
**       value)`.
*/
bool array_none(const Array* array, const void* value);

/*
** Check if no element of the array satisfies a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `true` if no element satisfies the predicate function, `false`
**         otherwise.
**
** @note This is equivalent to `array_noneWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
bool array_noneIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Check if no element of a specified range within the array is equal to a
** value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to compare to.
**
** @return `true` if no element within the range is equal to `value`, `false`
**         if not, or `ARRAY_FAILURE` in case of error.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
int array_noneWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Check if no element of a specified range within the array satisfies a
** predicate function.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `true` if no element within the range satisfies the predicate
**         function, `false` if not, or `ARRAY_FAILURE` in case of error.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
int array_noneWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Fill the array with a value.
**
** @param array The array.
** @param value The value to fill the array with.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `value` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
** @note This is equivalent to `array_fillWithin(array, 0, array_size(array),
**       value)`.
*/
int array_fill(Array* array, const void* value);

/*
** Fill a range within the array with a value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to fill the array with.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `value` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
*/
int array_fillWithin(Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Copy the elements of a buffer to the array.
**
** @param array The array.
** @param values The buffer to copy the elements from.
** @param count The number of elements to copy.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note This operation does not change the size of the array, it will only
**       overwrite the elements from the beginning, and copy as many elements as
**       possible, up to `count` (its behavior can be compared to `strncpy`).
** @note `values` must point to a buffer of at least `array_dataSize(array) *
**       max(count, array_size(array))` bytes.
** @note This is equivalent to `array_copyWithin(array, 0, array_size(array),
**       values, count)`.
*/
int array_copy(Array* array, const void* values, size_t count);

/*
** Copy the elements of a buffer to a range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param values The buffer to copy the elements from.
** @param count The number of elements to copy.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `values` must point to a buffer of at least `array_dataSize(array) *
**       max(count, to - from)` bytes.
** @note This operation does not change the size of the array, it will only
**       overwrite the elements from the beginning, and copy as many elements as
**       possible, up to `count` (its behavior can be compared to `strncpy`).
*/
int array_copyWithin(Array* array, ptrdiff_t from, ptrdiff_t to, const void* values, size_t count);

/*
** Remove the elements of the array that are equal to a value.
**
** @param array The array.
** @param value The value to remove.
**
** @note `value` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
** @note `memcmp` will be used to compare the elements.
** @note This is equivalent to `array_filterWithin(array, 0, array_size(array),
**       value)`.
*/
void array_filter(Array* array, const void* value);

/*
** Remove the elements of the array that satisfy a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @note This is equivalent to `array_filterWithinIf(array, 0,
**       array_size(array), predFn, param)`.
*/
void array_filterIf(Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Remove the elements of a specified range within the array that are equal to a
** value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to remove.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note `value` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
** @note `memcmp` will be used to compare the elements.
*/
int array_filterWithin(Array* array, ptrdiff_t from, ptrdiff_t t, const void* value);

/*
** Remove the elements of a specified range within the array that satisfy
** a predicate function.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
int array_filterWithinIf(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Create a new array resulting from the removal of the elements of the array
** that are equal to a value.
**
** @param array The array.
** @param value The value to remove.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note `value` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
** @note `memcmp` will be used to compare the elements.
*/
Array* array_filtered(const Array* array, const void* value);

/*
** Create a new array resulting from the removal of the elements of the array
** that satisfy a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note This is equivalent to `array_filteredWithinIf(array, 0,
**       array_size(array), predFn, param)`.
*/
Array* array_filteredIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Create a new array resulting from the removal of the elements of a
** specified range within the array that are equal to a value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to remove.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note `value` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
** @note `memcmp` will be used to compare the elements.
*/
Array* array_filteredWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Create a new array resulting from the removal of the elements of a
** specified range within the array that satisfy a predicate function.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
*/
Array* array_filteredWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn,
                              void* param);

/*
** Replace every occurrence of a value in the array with another value.
**
** @param array The array.
** @param oldValue The value to replace.
** @param newValue The value to replace with.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `memcmp` will be used to compare the elements.
** @note `oldValue` and `newValue` must point to a buffer of at least
**       `array_dataSize(array)` bytes.
** @note This is equivalent to `array_replaceWithin(array, 0, array_size(array),
**       oldValue, newValue)`.
*/
int array_replace(Array* array, const void* oldValue, const void* newValue);

/*
** Replace all elements of the array that satisfy a predicate function with
** another value.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
** @param newValue The value to replace with.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `newValue` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
** @note This is equivalent to `array_replaceWithinIf(array, 0,
**       array_size(array), predFn, param, newValue)`.
*/
int array_replaceIf(Array* array, ArrayPredicateFunction* predFn, void* param, const void* newValue);

/*
** Replace every occurrence of a value of a specified range within the array
** with another value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param oldValue The value to replace.
** @param newValue The value to replace with.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `memcmp` will be used to compare the elements.
** @note `oldValue` and `newValue` must point to a buffer of at least
**       `array_dataSize(array)` bytes.
*/
int array_replaceWithin(Array* array, ptrdiff_t from, ptrdiff_t to, const void* oldValue, const void* newValue);

/*
** Replace all elements of a specified range within the array that satisfy a
** predicate function with another value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
** @param newValue The value to replace with.
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `newValue` must point to a buffer of at least `array_dataSize(array)`
**       bytes.
*/
int array_replaceWithinIf(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param,
                          const void* newValue);

/*
** Create a new array resulting from the replacement of every occurrence of a
** value in the array with another value.
**
** @param array The array.
** @param oldValue The value to replace.
** @param newValue The value to replace with.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `memcmp` will be used to compare the elements.
** @note `oldValue` and `newValue` must point to a buffer of at least
**       `array_dataSize(array)` bytes.
** @note This is equivalent to `array_replacedWithin(array, 0,
** array_size(array), oldValue, newValue)`.
*/
Array* array_replaced(const Array* array, const void* oldValue, const void* newValue);

/*
** Create a new array resulting from the replacement of all elements of the
** array that satisfy a predicate function with another value.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
** @param newValue The value to replace with.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
*/
Array* array_replacedIf(const Array* array, ArrayPredicateFunction* predFn, void* param, const void* newValue);

/*
** Create a new array resulting from the replacement of every occurrence of a
** value of a specified range within the array with another value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param oldValue The value to replace.
** @param newValue The value to replace with.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
**
** @note `memcmp` will be used to compare the elements.
** @note `oldValue` and `newValue` must point to a buffer of at least
**       `array_dataSize(array)` bytes.
*/
Array* array_replacedWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* oldValue,
                            const void* newValue);

/*
** Create a new array resulting from the replacement of every occurrence of a
** value of a specified range within the array that don't satisfy a predicate
** function with another value.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
** @param newValue The value to replace with.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ECPYCTORFAIL` if the copy constructor failed.
*/
Array* array_replacedWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn,
                              void* param, const void* newValue);

/*
** Reverse the elements of the array.
**
** @param array The array.
*/
void array_reverse(Array* array);

/*
** Reverse the elements of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
int array_reverseWithin(Array* array, ptrdiff_t from, ptrdiff_t to);

/*
** Create a new array resulting from the reversal of the elements of the array.
**
** @param array The array.
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
*/
Array* array_reversed(const Array* array);

/*
** Create a new array resulting from the reversal of the elements of a specified
** range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
** @throws - `ARR_ERANGE` if the range is out of range.
*/
Array* array_reversedWithin(const Array* array, ptrdiff_t from, ptrdiff_t to);

/*
** Find the first occurrence of a value in the array.
**
** @param array The array.
** @param value The value to find.
**
** @return A pointer to the first occurrence of `value` in the array, or `NULL`
**         if not found.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_findWithin(array, 0, array_size(array),
**       value)`.
*/
void* array_find(Array* array, const void* value);

/*
** Find the first occurrence of a value in the array.
**
** @param array The array.
** @param value The value to find.
**
** @return A const pointer to the first occurrence of `value` in the array, or
**         `NULL` if not found.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is the const version of `array_find`.
** @note This is equivalent to `array_cFindWithin(array, 0, array_size(array),
**       value)`.
*/
const void* array_cFind(const Array* array, const void* value);

/*
** Find the last occurrence of a value in the array.
**
** @param array The array.
** @param value The value to find.
**
** @return A pointer to the last occurrence of `value` in the array, or `NULL`
**         if not found.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_rFindWithin(array, 0, array_size(array),
**       value)`.
*/
void* array_rFind(Array* array, const void* value);

/*
** Find the last occurrence of a value in the array.
**
** @param array The array.
** @param value The value to find.
**
** @return A const pointer to the last occurrence of `value` in the array, or
**         `NULL` if not found.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is the const version of `array_rFind`.
** @note This is equivalent to `array_crFindWithin(array, 0, array_size(array),
**       value)`.
*/
const void* array_crFind(const Array* array, const void* value);

/*
** Find the first element of the array that satisfies a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A pointer to the first element that satisfies the predicate function,
**         or `NULL` if not found.
**
** @note This is equivalent to `array_findWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
void* array_findIf(Array* array, ArrayPredicateFunction* predFn, void* param);

/* Find the first element of the array that satisfies a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A const pointer to the first element that satisfies the predicate
**         function, or `NULL` if not found.
**
** @note This is the const version of `array_findIf`.
** @note This is equivalent to `array_cFindWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
const void* array_cFindIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Find the last element of the array that satisfies a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A pointer to the last element that satisfies the predicate function,
**         or `NULL` if not found.
**
** @note This is equivalent to `array_rFindWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
void* array_rFindIf(Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Find the last element of the array that satisfies a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A const pointer to the last element that satisfies the predicate
**         function, or `NULL` if not found.
**
** @note This is the const version of `array_rFindIf`.
** @note This is equivalent to `array_crFindWithinIf(array, 0,
**       array_size(array), predFn, param)`.
*/
const void* array_crFindIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Find the first occurrence of a value in a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to find.
**
** @return A pointer to the first occurrence of `value` in the range, or `NULL`
**         if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
void* array_findWithin(Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Find the first occurrence of a value in a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to find.
**
** @return A const pointer to the first occurrence of `value` in the range, or
**         `NULL` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is the const version of `array_findWithin`.
*/
const void* array_cFindWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Find the last occurrence of a value in a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to find.
**
** @return A pointer to the last occurrence of `value` in the range, or `NULL`
**         if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
void* array_rFindWithin(Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Find the last occurrence of a value in a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to find.
**
** @return A const pointer to the last occurrence of `value` in the range, or
**         `NULL` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is the const version of `array_rFindWithin`.
*/
const void* array_crFindWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Find the first element of the array that satisfies a predicate function in a
** specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A pointer to the first element that satisfies the predicate function
**         in the range, or `NULL` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
void* array_findWithinIf(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Find the first element of the array that satisfies a predicate function in a
** specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A const pointer to the first element that satisfies the predicate
**         function in the range, or `NULL` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note This is the const version of `array_findWithinIf`.
*/
const void* array_cFindWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn,
                                void* param);

/*
** Find the last element of the array that satisfies a predicate function in a
** specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A pointer to the last element that satisfies the predicate function
**         in the range, or `NULL` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
void* array_rFindWithinIf(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Find the last element of the array that satisfies a predicate function in a
** specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return A const pointer to the last element that satisfies the predicate
**         function in the range, or `NULL` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note This is the const version of `array_rFindWithinIf`.
*/
const void* array_crFindWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn,
                                 void* param);

/*
** Find the index of the first occurrence of a value in the array.
**
** @param array The array.
** @param value The value to find.
**
** @return The index of the first occurrence of `value` in the array, or `-1` if
**         not found.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_indexWithin(array, 0, array_size(array),
**       value)`.
*/
ptrdiff_t array_index(const Array* array, const void* value);

/*
** Find the index of the last occurrence of a value in the array.
**
** @param array The array.
** @param value The value to find.
**
** @return The index of the last occurrence of `value` in the array, or `-1` if
**         not found.
**
** @note The function will use `memcmp` to compare the elements.
** @note This is equivalent to `array_rIndexWithin(array, 0, array_size(array),
**       value)`.
*/
ptrdiff_t array_rIndex(const Array* array, const void* value);

/*
** Find the index of the first element of the array that satisfies a predicate
** function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The index of the first element that satisfies the predicate function,
**         or `-1` if not found.
**
** @note This is equivalent to `array_indexWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
ptrdiff_t array_indexIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Find the index of the last element of the array that satisfies a predicate
** function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The index of the last element that satisfies the predicate function,
**         or `-1` if not found.
**
** @note This is equivalent to `array_rIndexWithinIf(array, 0,
**       array_size(array), predFn, param)`.
*/
ptrdiff_t array_rIndexIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Find the index of the first occurrence of a value in a specified range within
** the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to find.
**
** @return The index of the first occurrence of `value` in the range, or `-1` if
**         not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
ptrdiff_t array_indexWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Find the index of the last occurrence of a value in a specified range within
** the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to find.
**
** @return The index of the last occurrence of `value` in the range, or `-1` if
**         not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
**
** @note The function will use `memcmp` to compare the elements.
*/
ptrdiff_t array_rIndexWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Find the index of the first element of the array that satisfies a predicate
** function in a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The index of the first element that satisfies the predicate function
**         in the range, or `-1` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
ptrdiff_t array_indexWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn,
                              void* param);

/*
** Find the index of the last element of the array that satisfies a predicate
** function in a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The index of the last element that satisfies the predicate function
**         in the range, or `-1` if not found.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
*/
ptrdiff_t array_rIndexWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn,
                               void* param);

/*
** Rotate the elements of the array.
**
** @param array The array.
** @param n The number of positions to rotate the elements (positive values
**          rotate to the left, negative values rotate to the right).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note This is equivalent to `array_rotateWithin(array, 0, array_size(array),
**       n)`.
*/
int array_rotate(Array* array, ptrdiff_t n);

/*
** Rotate the elements of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param n The number of positions to rotate the elements (positive values
**          rotate to the left, negative values rotate to the right).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ENOMEM` if memory allocation failed.
*/
int array_rotateWithin(Array* array, ptrdiff_t from, ptrdiff_t to, ptrdiff_t n);

/*
** Create a new array resulting from the rotation of the elements of the array.
**
** @param array The array.
** @param n The number of positions to rotate the elements (positive values
**          rotate to the left, negative values rotate to the right).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ENOMEM` if memory allocation failed.
**
** @note This is equivalent to `array_rotatedWithin(array, 0, array_size(array),
**       n)`.
*/
Array* array_rotated(const Array* array, ptrdiff_t n);

/*
** Create a new array resulting from the rotation of the elements of a specified
** range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param n The number of positions to rotate the elements (positive values
**          rotate to the left, negative values rotate to the right).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is out of range.
** @throws - `ARR_ENOMEM` if memory allocation failed.
*/
Array* array_rotatedWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ptrdiff_t n);

/*
** Slice the array in place.
**
** @param array The array.
** @param from The starting position of the slice (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the slice, not included (if negative, it
**           will expand to `array_size(array) + to`).
**
** @return `ARRAY_SUCCESS` in case of success, `ARRAY_FAILURE` otherwise.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the slice is out of range.
*/
int array_slice(Array* array, ptrdiff_t from, ptrdiff_t to);

/*
** Get a slice of the array.
**
** @param array The array.
** @param from The starting position of the slice (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the slice, not included (if negative, it
**           will expand to `array_size(array) + to`).
**
** @return The new array (don't forget to free it with `array_destroy`), or
**         `NULL` in case of failure.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the slice is out of range.
** @throws - `ARR_ENOMEM` if memory allocation failed.
*/
Array* array_sliced(const Array* array, ptrdiff_t from, ptrdiff_t to);

/*
** Get the number of occurrences of a value in the array.
**
** @param array The array.
** @param value The value to count.
**
** @return The number of occurrences of `value` in the array.
**
** @note `memcmp` will be used to compare the elements.
** @note This is equivalent to `array_countWithin(array, 0, array_size(array),
**       value)`.
*/
int array_count(const Array* array, const void* value);

/*
** Get the number of elements of the array that satisfy a predicate function.
**
** @param array The array.
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The number of elements that satisfy the predicate function in the
**         array.
**
** @note This is equivalent to `array_countWithinIf(array, 0, array_size(array),
**       predFn, param)`.
*/
int array_countIf(const Array* array, ArrayPredicateFunction* predFn, void* param);

/*
** Get the number of occurrences of a value in a specified range within the
** array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param value The value to count.
**
** @return The number of occurrences of `value` in the range.
**
** @note `memcmp` will be used to compare the elements.
*/
int array_countWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, const void* value);

/*
** Get the number of elements of the array that satisfy a predicate function in
** a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param predFn The predicate function to use.
** @param param An additional parameter to pass to the predicate function (can
**              be `NULL`).
**
** @return The number of elements that satisfy the predicate function in the
**         range.
*/
int array_countWithinIf(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayPredicateFunction* predFn, void* param);

/*
** Get the minimum element of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A pointer to the minimum element of the array, or `NULL` if the array
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is equivalent to `array_minWithin(array, 0, array_size(array),
**       cmpFn, param)`.
*/
void* array_min(Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the minimum element of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A const pointer to the minimum element of the array, or `NULL` if the
**         array is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is the const version of `array_min`.
** @note This is equivalent to `array_cMinWithin(array, 0, array_size(array),
**       cmpFn, param)`.
*/
const void* array_cMin(const Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the minimum element of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A pointer to the minimum element of the range, or `NULL` if the range
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is empty.
*/
void* array_minWithin(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the minimum element of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A const pointer to the minimum element of the range, or `NULL` if the
**         range is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is empty.
**
** @note This is the const version of `array_minWithin`.
*/
const void* array_cMinWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn,
                             void* param);

/*
** Get the index of the minimum element of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return The index of the minimum element of the array, or `-1` if the array
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is equivalent to `array_minIndexWithin(array, 0,
**       array_size(array), cmpFn, param)`.
*/
ptrdiff_t array_minIndex(const Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the index of the minimum element of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return The index of the minimum element of the range, or `-1` if the range
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is empty.
*/
ptrdiff_t array_minIndexWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn,
                               void* param);

/*
** Get the maximum element of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A pointer to the maximum element of the array, or `NULL` if the array
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is equivalent to `array_maxWithin(array, 0, array_size(array),
**       cmpFn, param)`.
*/
void* array_max(Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the maximum element of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A const pointer to the maximum element of the array, or `NULL` if the
**         array is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is the const version of `array_max`.
** @note This is equivalent to `array_cMaxWithin(array, 0, array_size(array),
**       cmpFn, param)`.
*/
const void* array_cMax(const Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the maximum element of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A pointer to the maximum element of the range, or `NULL` if the range
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is empty.
*/
void* array_maxWithin(Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the maximum element of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return A const pointer to the maximum element of the range, or `NULL` if the
**         range is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is empty.
**
** @note This is the const version of `array_maxWithin`.
*/
const void* array_cMaxWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn,
                             void* param);

/*
** Get the index of the maximum element of the array.
**
** @param array The array.
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return The index of the maximum element of the array, or `-1` if the array
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the array is empty.
**
** @note This is equivalent to `array_maxIndexWithin(array, 0,
**       array_size(array), cmpFn, param)`.
*/
ptrdiff_t array_maxIndex(const Array* array, ArrayCompareFunction* cmpFn, void* param);

/*
** Get the index of the maximum element of a specified range within the array.
**
** @param array The array.
** @param from The starting position of the range (if negative, it will expand
**             to `array_size(array) + from`).
** @param to The ending position of the range, not included (if negative, it
**           will expand to `array_size(array) + to`).
** @param cmpFn The comparison function to use.
** @param param An additional parameter to pass to the comparison function (can
**              be `NULL`).
**
** @return The index of the maximum element of the range, or `-1` if the range
**         is empty.
**
** @throws Error codes (accessed via `array_errno`):
** @throws - `ARR_ERANGE` if the range is empty.
*/
ptrdiff_t array_maxIndexWithin(const Array* array, ptrdiff_t from, ptrdiff_t to, ArrayCompareFunction* cmpFn,
                               void* param);

#endif
