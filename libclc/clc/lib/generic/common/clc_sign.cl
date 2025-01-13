#include <clc/clcmacro.h>
#include <clc/internal/clc.h>
#include <clc/relational/clc_isnan.h>

#define CLC_SIGN(TYPE, F)                                                      \
  _CLC_DEF _CLC_OVERLOAD TYPE __clc_sign(TYPE x) {                             \
    if (__clc_isnan(x)) {                                                      \
      return 0.0F;                                                             \
    }                                                                          \
    if (x > 0.0F) {                                                            \
      return 1.0F;                                                             \
    }                                                                          \
    if (x < 0.0F) {                                                            \
      return -1.0F;                                                            \
    }                                                                          \
    return x; /* -0.0 or +0.0 */                                               \
  }

CLC_SIGN(float, f)
_CLC_UNARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, float, __clc_sign, float)

#ifdef cl_khr_fp64

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

CLC_SIGN(double, )
_CLC_UNARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, double, __clc_sign, double)

#endif

#ifdef cl_khr_fp16

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

CLC_SIGN(half, )
_CLC_UNARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, half, __clc_sign, half)

#endif
