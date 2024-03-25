#define ARRAY_USE_IMPL
#include <array.h>

void array_clear(Array * arr) {
  array_getFactory(arr)->destructor(arr, array_data(arr), array_size(arr));
  array_setSize(arr, 0);
}
