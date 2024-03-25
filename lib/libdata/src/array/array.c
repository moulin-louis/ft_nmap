#define ARRAY_IMPL
#include <array.h>

Array * array(
    size_t dataSize,
    size_t capacity,
    size_t size,
    const void * data,
    const ArrayFactory * factory
) {
  assert(dataSize != 0 && "dataSize must be greater than 0");

  capacity = max(size, capacity);

  ArrayFactory fact = array_factory(factory);

  Array * array = malloc(sizeof(Array));
  uint8_t * arrayData = fact.allocator(capacity * dataSize);

  if (!array || !arrayData)
    goto NoMemoryError;
  array->dataSize = dataSize;
  array->factory = fact;
  array->capacity = capacity;
  array->size = size;
  array->data = arrayData;
  if (data) {
    if (fact.copyConstructor(array, array->data, data, array->size))
      goto CopyConstructorError;
  } else if (fact.constructor(array, array->data, array->size))
    goto ConstructorError;

  return array;

NoMemoryError:

  array_errno = ARR_ENOMEM;
  goto AnyError;

CopyConstructorError:

  array_errno = ARR_ECPYCTORFAIL;
  goto AnyError;

ConstructorError:

  array_errno = ARR_ECTORFAIL;

AnyError:

  fact.deallocator(arrayData);
  free(array);
  return NULL;
}
