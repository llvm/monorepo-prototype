#include <clc/internal/clc.h>

#define __CLC_UPSAMPLE_IMPL(BGENTYPE, GENTYPE, UGENTYPE, GENSIZE)              \
  _CLC_OVERLOAD _CLC_DEF BGENTYPE __clc_upsample(GENTYPE hi, UGENTYPE lo) {    \
    return ((BGENTYPE)hi << GENSIZE) | lo;                                     \
  }                                                                            \
  _CLC_OVERLOAD _CLC_DEF BGENTYPE##2 __clc_upsample(GENTYPE##2 hi,             \
                                                    UGENTYPE##2 lo) {          \
    return (BGENTYPE##2){__clc_upsample(hi.s0, lo.s0),                         \
                         __clc_upsample(hi.s1, lo.s1)};                        \
  }                                                                            \
  _CLC_OVERLOAD _CLC_DEF BGENTYPE##3 __clc_upsample(GENTYPE##3 hi,             \
                                                    UGENTYPE##3 lo) {          \
    return (BGENTYPE##3){__clc_upsample(hi.s0, lo.s0),                         \
                         __clc_upsample(hi.s1, lo.s1),                         \
                         __clc_upsample(hi.s2, lo.s2)};                        \
  }                                                                            \
  _CLC_OVERLOAD _CLC_DEF BGENTYPE##4 __clc_upsample(GENTYPE##4 hi,             \
                                                    UGENTYPE##4 lo) {          \
    return (BGENTYPE##4){__clc_upsample(hi.lo, lo.lo),                         \
                         __clc_upsample(hi.hi, lo.hi)};                        \
  }                                                                            \
  _CLC_OVERLOAD _CLC_DEF BGENTYPE##8 __clc_upsample(GENTYPE##8 hi,             \
                                                    UGENTYPE##8 lo) {          \
    return (BGENTYPE##8){__clc_upsample(hi.lo, lo.lo),                         \
                         __clc_upsample(hi.hi, lo.hi)};                        \
  }                                                                            \
  _CLC_OVERLOAD _CLC_DEF BGENTYPE##16 __clc_upsample(GENTYPE##16 hi,           \
                                                     UGENTYPE##16 lo) {        \
    return (BGENTYPE##16){__clc_upsample(hi.lo, lo.lo),                        \
                          __clc_upsample(hi.hi, lo.hi)};                       \
  }

#define __CLC_UPSAMPLE_TYPES()                                                 \
  __CLC_UPSAMPLE_IMPL(short, char, uchar, 8)                                   \
  __CLC_UPSAMPLE_IMPL(ushort, uchar, uchar, 8)                                 \
  __CLC_UPSAMPLE_IMPL(int, short, ushort, 16)                                  \
  __CLC_UPSAMPLE_IMPL(uint, ushort, ushort, 16)                                \
  __CLC_UPSAMPLE_IMPL(long, int, uint, 32)                                     \
  __CLC_UPSAMPLE_IMPL(ulong, uint, uint, 32)

__CLC_UPSAMPLE_TYPES()

#undef __CLC_UPSAMPLE_TYPES
#undef __CLC_UPSAMPLE_IMPL
