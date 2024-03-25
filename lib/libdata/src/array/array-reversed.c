#define ARRAY_USE_IMPL
#include <array.h>

Array * array_reversed(const Array * arr) {
  return array_reversedWithin(arr, 0, array_size(arr));
}

Array * array_reversedWithin(const Array * arr, ptrdiff_t from, ptrdiff_t to) {
  assert(arr != NULL && "array cannot be NULL");

  if (array_setupRange(arr, &from, &to))
    return NULL;

  const size_t toBackup = to;

  Array * dst = array(
      array_dataSize(arr), array_size(arr), 0, NULL, array_getFactory(arr)
  );

  if (!dst)
    goto AnyError;

  if (array_getFactory(dst)->copyConstructor(
          dst, array_data(dst), array_cData(arr), from
      ))
    goto CopyConstructorError;
  while (from < --to) {
    if (array_getFactory(dst)->copyConstructor(
            dst, array_dataOffset(dst, from), array_cDataOffset(arr, to), 1
        ))
      goto LoopCopyConstructorError;
    ++from;
    if (array_getFactory(dst)->copyConstructor(
            dst,
            array_dataOffset(dst, to),
            array_cDataOffset((void *)arr, from - 1),
            1
        ))
      goto LoopCopyConstructorError;
  }
  if (from == to &&
      array_getFactory(dst)->copyConstructor(
          dst, array_dataOffset(dst, from), array_cDataOffset(arr, to), 1
      ))
    goto LoopCopyConstructorError;

  if (array_getFactory(dst)->copyConstructor(
          dst,
          array_dataOffset(dst, toBackup),
          array_cDataOffset(arr, toBackup),
          (array_size(arr) - toBackup)
      ))
    goto RightCopyConstructorError;

  array_setSize(dst, array_size(arr));
  return dst;

LoopCopyConstructorError:
  array_getFactory(dst)->destructor(dst, array_data(dst), from);
  array_getFactory(dst)->destructor(
      dst, array_dataOffset(dst, to + 1), array_size(arr) - to - 1
  );
  goto CopyConstructorError;

RightCopyConstructorError:

  array_getFactory(dst)->destructor(dst, array_data(dst), array_size(arr) - to);

CopyConstructorError:
  array_errno = ARR_ECPYCTORFAIL;

AnyError:

  array_destroy(dst);
  return NULL;
}
