#ifndef __CLC_INTEGER_CLC_MAD24_H__
#define __CLC_INTEGER_CLC_MAD24_H__

#if defined(CLC_CLSPV) || defined(CLC_SPIRV)
// clspv and spir-v targets provide their own OpenCL-compatible mad24
#define __clc_mad24 mad24
#else

#include <clc/clcfunc.h>
#include <clc/clctypes.h>

#define FUNCTION __clc_mad24
#define __CLC_BODY "ternary_decl.h"

#include <clc/integer/gentype24.inc>

#undef FUNCTION

#endif

#endif // __CLC_INTEGER_CLC_MAD24_H__
