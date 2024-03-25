#include <array.h>

Array * array_clone(const Array * arr) {
  return array(
      array_dataSize(arr),
      array_capacity(arr),
      array_size(arr),
      array_cData(arr),
      array_getFactory(arr)
  );
}
