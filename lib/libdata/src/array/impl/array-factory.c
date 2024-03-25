#define ARRAY_USE_IMPL
#include <array.h>

ArrayFactory array_factory(const ArrayFactory * input) {
  static const ArrayFactory defaultFactory = {
      .allocator = malloc,
      .deallocator = free,
      .reallocator = realloc,
      .constructor = array_defaultCtor,
      .copyConstructor = array_defaultCopyCtor,
      .destructor = array_defaultDtor,
  };
  if (!input)
    return defaultFactory;

  return (ArrayFactory){
      .allocator =
          input->allocator ? input->allocator : defaultFactory.allocator,
      .deallocator =
          input->deallocator ? input->deallocator : defaultFactory.deallocator,
      .reallocator =
          input->reallocator ? input->reallocator : defaultFactory.reallocator,
      .constructor =
          input->constructor ? input->constructor : defaultFactory.constructor,
      .copyConstructor = input->copyConstructor
                             ? input->copyConstructor
                             : defaultFactory.copyConstructor,
      .destructor =
          input->destructor ? input->destructor : defaultFactory.destructor,
  };
}
