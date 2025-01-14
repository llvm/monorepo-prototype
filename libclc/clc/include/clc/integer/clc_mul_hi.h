#ifndef __CLC_INTEGER_CLC_MUL_HI_H__
#define __CLC_INTEGER_CLC_MUL_HI_H__

#if defined(CLC_CLSPV) || defined(CLC_SPIRV)
// clspv and spir-v targets provide their own OpenCL-compatible mul_hi
#define __clc_mul_hi mul_hi
#else

#include <clc/clcfunc.h>
#include <clc/clctypes.h>

#define FUNCTION __clc_mul_hi
#define __CLC_BODY "binary_decl.h"

#include <clc/integer/gentype.inc>

#undef FUNCTION

#endif

#endif // __CLC_INTEGER_CLC_MUL_HI_H__
