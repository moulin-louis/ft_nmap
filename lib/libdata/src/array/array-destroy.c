#define ARRAY_USE_IMPL
#include <array.h>

void array_destroy(Array * arr) {
  if (arr) {
    array_getFactory(arr)->destructor(arr, array_data(arr), array_size(arr));
    array_getFactory(arr)->deallocator(array_data(arr));
    free(arr);
  }
}
