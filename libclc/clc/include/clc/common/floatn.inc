#define __CLC_FLOATN float
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN float2
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN float3
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN float4
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN float8
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN float16
#include __CLC_BODY
#undef __CLC_FLOATN

#undef __CLC_FLOAT
#undef __CLC_INT

#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define __CLC_FLOATN double
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN double2
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN double3
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN double4
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN double8
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN double16
#include __CLC_BODY
#undef __CLC_FLOATN

#endif
#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define __CLC_FLOATN half
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN half2
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN half3
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN half4
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN half8
#include __CLC_BODY
#undef __CLC_FLOATN

#define __CLC_FLOATN half16
#include __CLC_BODY
#undef __CLC_FLOATN

#endif

#undef __CLC_BODY
