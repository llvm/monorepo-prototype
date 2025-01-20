#ifndef __CLC_INTEGER_CLC_POPCOUNT_H__
#define __CLC_INTEGER_CLC_POPCOUNT_H__

#if defined(CLC_CLSPV) || defined(CLC_SPIRV)
// clspv and spir-v targets provide their own OpenCL-compatible popcount
#define __clc_popcount popcount
#else

// Map the function to an LLVM intrinsic
#define __CLC_FUNCTION __clc_popcount
#define __CLC_INTRINSIC "llvm.ctpop"
#include <clc/integer/unary_intrin.inc>

#undef __CLC_INTRINSIC
#undef __CLC_FUNCTION

#endif

#endif // __CLC_INTEGER_CLC_POPCOUNT_H__
