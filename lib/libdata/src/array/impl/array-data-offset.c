#include <array.h>

void * array_dataOffset(Array * arr, ptrdiff_t pos) {
  return (uint8_t *)array_data(arr) + pos * array_dataSize(arr);
}

const void * array_cDataOffset(const Array * arr, ptrdiff_t pos) {
  return array_dataOffset((void *)arr, pos);
}
