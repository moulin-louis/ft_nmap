#define ARRAY_IMPL
#include <array.h>

int array_emplaceBack(Array * arr, size_t count) {
  return array_emplace(arr, array_size(arr), count);
}
