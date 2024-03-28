#define ARRAY_USE_IMPL
#include <array.h>

_Atomic uint8_t * array_errno_location(void) {
  static _Atomic uint8_t err = ARRAY_SUCCESS;
  return &err;
}
