#ifndef __CLC_INTEGER_CLC_MUL24_H__
#define __CLC_INTEGER_CLC_MUL24_H__

#if defined(CLC_CLSPV) || defined(CLC_SPIRV)
// clspv and spir-v targets provide their own OpenCL-compatible mul24
#define __clc_mul24 mul24
#else

#include <clc/clcfunc.h>
#include <clc/clctypes.h>

#define FUNCTION __clc_mul24
#define __CLC_BODY "binary_decl.h"

#include <clc/integer/gentype24.inc>

#undef FUNCTION

#endif

#endif // __CLC_INTEGER_CLC_MUL24_H__
