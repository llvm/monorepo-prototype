#include <clc/clc.h>
#include <clc/clcmacro.h>
#include <clc/common/clc_sign.h>

_CLC_DEFINE_UNARY_BUILTIN(float, sign, __clc_sign, float)

#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

_CLC_DEFINE_UNARY_BUILTIN(double, sign, __clc_sign, double)

#endif

#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

_CLC_DEFINE_UNARY_BUILTIN(half, sign, __clc_sign, half)

#endif
