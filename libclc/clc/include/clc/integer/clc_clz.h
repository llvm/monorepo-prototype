#ifndef __CLC_INTEGER_CLC_CLZ_H__
#define __CLC_INTEGER_CLC_CLZ_H__

#if defined(CLC_CLSPV) || defined(CLC_SPIRV)
// clspv and spir-v targets provide their own OpenCL-compatible clz
#define __clc_clz clz
#else

#include <clc/clcfunc.h>
#include <clc/clctypes.h>

#define FUNCTION __clc_clz
#define __CLC_BODY "unary_decl.h"

#include <clc/integer/gentype.inc>

#undef FUNCTION

#endif

#endif // __CLC_INTEGER_CLC_CLZ_H__
