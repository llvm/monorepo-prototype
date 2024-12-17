#ifndef __CLC_INTEGER_CLC_HADD_H__
#define __CLC_INTEGER_CLC_HADD_H__

#if defined(CLC_CLSPV) || defined(CLC_SPIRV)
// clspv and spir-v targets provide their own OpenCL-compatible hadd
#define __clc_hadd hadd
#else

#include <clc/clcfunc.h>
#include <clc/clctypes.h>

#define FUNCTION __clc_hadd
#define __CLC_BODY "binary_decl.h"

#include <clc/integer/gentype.inc>

#undef FUNCTION

#endif

#endif // __CLC_INTEGER_CLC_HADD_H__
