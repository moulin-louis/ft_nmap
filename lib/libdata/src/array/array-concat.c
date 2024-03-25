#include <array.h>

Array * array_concat(
    const Array * lhs, const Array * rhs, const ArrayFactory * factory
) {
  Array * arr = NULL;

  if (array_dataSize(lhs) != array_dataSize(rhs))
    goto DataSizesNotSameError;

  if (!(arr = array(
            array_dataSize(lhs),
            array_size(lhs) + array_size(rhs),
            array_size(lhs),
            array_cData(lhs),
            factory
        )) ||
      array_pushBack(arr, array_cData(rhs), array_size(rhs)))
    goto AnyError;

  return arr;

DataSizesNotSameError:

  array_errno = ARR_EDIFFDATASIZE;

AnyError:

  array_destroy(arr);
  return NULL;
}
