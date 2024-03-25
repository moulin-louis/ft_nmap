#define ARRAY_IMPL
#include <array.h>

const char * array_strerror(void) {
  static const char * errorMsgs[] = {
      [ARR_SUCCESS] = "Success",
      [ARR_ENOMEM] = "Out of memory",
      [ARR_ECTORFAIL] = "Constructor function returned a non-zero value",
      [ARR_ECPYCTORFAIL] =
          "Copy-constructor function returned a non-zero value",
      [ARR_ERANGE] = "Index out of range",
      [ARR_EITERFAIL] = "Array iter function returned a non-zero value",
      [ARR_EMAPFAIL] = "Array map function returned a non-zero value",
      [ARR_EDIFFDATASIZE] = "Data sizes of arrays are different",
  };
  if (array_errno >= sizeof(errorMsgs) / sizeof(*errorMsgs))
    return "Unknown error";
  return errorMsgs[array_errno];
}
