#include <array.h>

uint8_t * array_errno_location(void) {
  static uint8_t err = ARRAY_SUCCESS;
  return &err;
}
