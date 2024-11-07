#ifndef __CLC_COMMON_CLC_SIGN_H__
#define __CLC_COMMON_CLC_SIGN_H__

#if defined(CLC_CLSPV)
// clspv targets provide their own OpenCL-compatible sign
#define __clc_sign sign
#else

#define __CLC_FUNCTION __clc_sign
#define __CLC_BODY <clc/math/unary_decl.inc>
#include <clc/math/gentype.inc>
#undef __CLC_FUNCTION
#undef __CLC_BODY

#endif

#endif // __CLC_COMMON_CLC_SIGN_H__
