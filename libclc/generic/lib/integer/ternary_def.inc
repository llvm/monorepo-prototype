#include <clc/utils.h>

#define __CLC_FUNCTION(x) __CLC_CONCAT(__clc_, x)

_CLC_OVERLOAD _CLC_DEF __CLC_GENTYPE FUNCTION(__CLC_GENTYPE a, __CLC_GENTYPE b,
                                              __CLC_GENTYPE c) {
  return __CLC_FUNCTION(FUNCTION)(a, b, c);
}
