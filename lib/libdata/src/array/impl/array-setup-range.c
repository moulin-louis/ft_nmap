#define ARRAY_IMPL
#include <array.h>

int array_setupRange(const Array * arr, ptrdiff_t * from, ptrdiff_t * to) {
  if (array_setupPos(arr, from) || array_setupPos(arr, to))
    return ARRAY_FAILURE;

  if (*from > *to)
    *from = *to;

  return ARRAY_SUCCESS;
}
