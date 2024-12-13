/*===--------------- avx10_2convertintrin.h - AVX10_2CONVERT ---------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error                                                                         \
    "Never use <avx10_2convertintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifdef __SSE2__

#ifndef __AVX10_2CONVERTINTRIN_H
#define __AVX10_2CONVERTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avx10.2-256"),    \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avx10.2-256"),    \
                 __min_vector_width__(256)))

/// Convert two 128-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 128-bit vector
/// containing FP16 elements.
///
/// \code{.operation]
/// FOR i := 0 to 7
/// 	IF i < 4
/// 		dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 	ELSE
/// 		dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 4])
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __A
///    A 128-bit vector of [4 x float].
/// \param __B
///    A 128-bit vector of [4 x float].
/// \returns
///    A 128-bit vector of [8 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A.
static __inline__ __m128h __DEFAULT_FN_ATTRS128 _mm_cvtx2ps_ph(__m128 __A,
                                                               __m128 __B) {
  return (__m128h)__builtin_ia32_vcvt2ps2phx128_mask(
      (__v4sf)__A, (__v4sf)__B, (__v8hf)_mm_setzero_ph(), (__mmask8)(-1));
}

/// Convert two 128-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 128-bit vector
/// containing FP16 elements. Merging mask \a __U is used to determine if given
/// element should be taken from \a __W instead.
///
/// \code{.operation]
/// FOR i := 0 to 7
/// 	IF mask[i]
/// 		dst.fp16[i] := __W[i]
/// 	ELSE
/// 		IF i < 4
/// 			dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 		ELSE
/// 			dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 4])
/// 		FI
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __W
///    A 128-bit vector of [8 x fp16].
/// \param __U
///    A 8-bit merging mask.
/// \param __A
///    A 128-bit vector of [4 x float].
/// \param __B
///    A 128-bit vector of [4 x float].
/// \returns
///    A 128-bit vector of [8 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A. If corresponding mask bit is set, then
///    element from \a __W is taken instead.
static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_cvtx2ps_ph(__m128h __W, __mmask8 __U, __m128 __A, __m128 __B) {
  return (__m128h)__builtin_ia32_vcvt2ps2phx128_mask(
      (__v4sf)__A, (__v4sf)__B, (__v8hf)__W, (__mmask8)__U);
}

/// Convert two 128-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 128-bit vector
/// containing FP16 elements. Zeroing mask \a __U is used to determine if given
/// element should be zeroed instead.
///
/// \code{.operation]
/// FOR i := 0 to 7
/// 	IF mask[i]
/// 		dst.fp16[i] := 0
/// 	ELSE
/// 		IF i < 4
/// 			dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 		ELSE
/// 			dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 4])
/// 		FI
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __U
///    A 8-bit merging mask.
/// \param __A
///    A 128-bit vector of [4 x float].
/// \param __B
///    A 128-bit vector of [4 x float].
/// \returns
///    A 128-bit vector of [8 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A. If corresponding mask bit is set, then
///    zero is taken instead.
static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_cvtx2ps_ph(__mmask8 __U, __m128 __A, __m128 __B) {
  return (__m128h)__builtin_ia32_vcvt2ps2phx128_mask(
      (__v4sf)__A, (__v4sf)__B, (__v8hf)_mm_setzero_ph(), (__mmask8)__U);
}

/// Convert two 256-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 256-bit vector
/// containing FP16 elements.
///
/// \code{.operation]
/// FOR i := 0 to 15 
/// 	IF i < 8
/// 		dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 	ELSE
/// 		dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 8])
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __A
///    A 256-bit vector of [8 x float].
/// \param __B
///    A 256-bit vector of [8 x float].
/// \returns
///    A 256-bit vector of [16 x fp16]. Lower 8 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A.
static __inline__ __m256h __DEFAULT_FN_ATTRS256 _mm256_cvtx2ps_ph(__m256 __A,
                                                                  __m256 __B) {
  return (__m256h)__builtin_ia32_vcvt2ps2phx256_mask(
      (__v8sf)__A, (__v8sf)__B, (__v16hf)_mm256_setzero_ph(), (__mmask16)(-1),
      _MM_FROUND_CUR_DIRECTION);
}

/// Convert two 256-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 256-bit vector
/// containing FP16 elements. Merging mask \a __U is used to determine if given
/// element should be taken from \a __W instead.
///
/// \code{.operation]
/// FOR i := 0 to 15
/// 	IF mask[i]
/// 		dst.fp16[i] := __W[i]
/// 	ELSE
/// 		IF i < 8
/// 			dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 		ELSE
/// 			dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 8])
/// 		FI
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __W
///    A 256-bit vector of [16 x fp16].
/// \param __U
///    A 8-bit merging mask.
/// \param __A
///    A 256-bit vector of [8 x float].
/// \param __B
///    A 256-bit vector of [8 x float].
/// \returns
///    A 256-bit vector of [16 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A. If corresponding mask bit is set, then
///    element from \a __W is taken instead.
static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_cvtx2ps_ph(__m256h __W, __mmask16 __U, __m256 __A, __m256 __B) {
  return (__m256h)__builtin_ia32_vcvt2ps2phx256_mask(
      (__v8sf)__A, (__v8sf)__B, (__v16hf)__W, (__mmask16)__U,
      _MM_FROUND_CUR_DIRECTION);
}

/// Convert two 256-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 256-bit vector
/// containing FP16 elements. Zeroing mask \a __U is used to determine if given
/// element should be zeroed instead.
///
/// \code{.operation]
/// FOR i := 0 to 15 
/// 	IF mask[i]
/// 		dst.fp16[i] := 0
/// 	ELSE
/// 		IF i < 8
/// 			dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 		ELSE
/// 			dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 8])
/// 		FI
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __U
///    A 8-bit merging mask.
/// \param __A
///    A 256-bit vector of [8 x float].
/// \param __B
///    A 256-bit vector of [8 x float].
/// \returns
///    A 256-bit vector of [16 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A. If corresponding mask bit is set, then
///    zero is taken instead.
static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtx2ps_ph(__mmask16 __U, __m256 __A, __m256 __B) {
  return (__m256h)__builtin_ia32_vcvt2ps2phx256_mask(
      (__v8sf)__A, (__v8sf)__B, (__v16hf)_mm256_setzero_ph(), (__mmask16)__U,
      _MM_FROUND_CUR_DIRECTION);
}

/// Convert two 256-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 256-bit vector
/// containing FP16 elements. Rounding mode \a __R needs to be provided.
///
/// \code{.operation]
/// FOR i := 0 to 15 
/// 	IF i < 8
/// 		dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 	ELSE
/// 		dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 8])
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __A
///    A 256-bit vector of [8 x float].
/// \param __B
///    A 256-bit vector of [8 x float].
/// \param __R
///    Rounding mode. Valid inputs are: _MM_FROUND_CUR_DIRECTION or
///    result bitwise or of _MM_FROUND_NO_EXC with at most one of the following:
///    _MM_FROUND_TO_NEAREST_INT, _MM_FROUND_TO_NEG_INF, _MM_FROUND_TO_POS_INF,
///    _MM_FROUND_TO_ZERO.
/// \returns
///    A 256-bit vector of [16 x fp16]. Lower 8 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A.
#define _mm256_cvtx_round2ps_ph(A, B, R)                                       \
  ((__m256h)__builtin_ia32_vcvt2ps2phx256_mask(                                \
      (__v8sf)(A), (__v8sf)(B), (__v16hf)_mm256_undefined_ph(),                \
      (__mmask16)(-1), (const int)(R)))

/// Convert two 256-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 256-bit vector
/// containing FP16 elements. Merging mask \a __U is used to determine if given
/// element should be taken from \a __W instead. Rounding mode \a __R needs to
/// be provided.
///
/// \code{.operation]
/// FOR i := 0 to 15
/// 	IF mask[i]
/// 		dst.fp16[i] := __W[i]
/// 	ELSE
/// 		IF i < 8
/// 			dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 		ELSE
/// 			dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 8])
/// 		FI
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __W
///    A 256-bit vector of [16 x fp16].
/// \param __U
///    A 8-bit merging mask.
/// \param __A
///    A 256-bit vector of [8 x float].
/// \param __B
///    A 256-bit vector of [8 x float].
/// \param __R
///    Rounding mode. Valid inputs are: _MM_FROUND_CUR_DIRECTION or
///    result bitwise or of _MM_FROUND_NO_EXC with at most one of the following:
///    _MM_FROUND_TO_NEAREST_INT, _MM_FROUND_TO_NEG_INF, _MM_FROUND_TO_POS_INF,
///    _MM_FROUND_TO_ZERO.
/// \returns
///    A 256-bit vector of [16 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A. If corresponding mask bit is set, then
///    element from \a __W is taken instead.
#define _mm256_mask_cvtx_round2ps_ph(W, U, A, B, R)                            \
  ((__m256h)__builtin_ia32_vcvt2ps2phx256_mask(                                \
      (__v8sf)(A), (__v8sf)(B), (__v16hf)(W), (__mmask16)(U), (const int)(R)))

/// Convert two 256-bit vectors, \a __A and \a __B, containing packed
/// single-precision (32-bit) floating-point elements to a 256-bit vector
/// containing FP16 elements. Zeroing mask \a __U is used to determine if given
/// element should be zeroed instead. Rounding mode \a __R needs to be provided.
///
/// \code{.operation]
/// FOR i := 0 to 15 
/// 	IF mask[i]
/// 		dst.fp16[i] := 0
/// 	ELSE
/// 		IF i < 8
/// 			dst.fp16[i] := convert_fp32_to_fp16(__B.fp32[i])
/// 		ELSE
/// 			dst.fp16[i] := convert_fp32_to_fp16(__A.fp32[i - 8])
/// 		FI
/// 	FI
/// ENDFOR
/// \endcode
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the \c VCVT2PS2PHX instruction.
///
/// \param __U
///    A 8-bit merging mask.
/// \param __A
///    A 256-bit vector of [8 x float].
/// \param __B
///    A 256-bit vector of [8 x float].
/// \param __R
///    Rounding mode. Valid inputs are: _MM_FROUND_CUR_DIRECTION or
///    result bitwise or of _MM_FROUND_NO_EXC with at most one of the following:
///    _MM_FROUND_TO_NEAREST_INT, _MM_FROUND_TO_NEG_INF, _MM_FROUND_TO_POS_INF,
///    _MM_FROUND_TO_ZERO.
/// \returns
///    A 256-bit vector of [16 x fp16]. Lower 4 elements correspond to the
///    (converted) elements from \a __B; higher order elements correspond to the
///    (converted) elements from \a __A. If corresponding mask bit is set, then
///    zero is taken instead.
#define _mm256_maskz_cvtx_round2ps_ph(U, A, B, R)                              \
  ((__m256h)__builtin_ia32_vcvt2ps2phx256_mask(                                \
      (__v8sf)(A), (__v8sf)(B), (__v16hf)(_mm256_setzero_ph()),                \
      (__mmask16)(U), (const int)(R)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiasph_pbf8(__m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiasph_pbf8(__m128i __W, __mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiasph_pbf8(__mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiasph_pbf8(__m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_undefined_si128(),
      (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtbiasph_pbf8(
    __m128i __W, __mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiasph_pbf8(__mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiassph_pbf8(__m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiassph_pbf8(__m128i __W, __mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiassph_pbf8(__mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiassph_pbf8(__m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_undefined_si128(),
      (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtbiassph_pbf8(
    __m128i __W, __mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiassph_pbf8(__mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiasph_phf8(__m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiasph_phf8(__m128i __W, __mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiasph_phf8(__mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiasph_phf8(__m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_undefined_si128(),
      (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtbiasph_phf8(
    __m128i __W, __mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiasph_phf8(__mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiassph_phf8(__m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiassph_phf8(__m128i __W, __mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiassph_phf8(__mmask8 __U, __m128i __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_128_mask(
      (__v16qi)__A, (__v8hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiassph_phf8(__m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_undefined_si128(),
      (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtbiassph_phf8(
    __m128i __W, __mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiassph_phf8(__mmask16 __U, __m256i __A, __m256h __B) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_256_mask(
      (__v32qi)__A, (__v16hf)__B, (__v16qi)(__m128i)_mm_setzero_si128(),
      (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_cvtne2ph_pbf8(__m128h __A,
                                                                  __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2bf8_128((__v8hf)(__A),
                                                   (__v8hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtne2ph_pbf8(__m128i __W, __mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtne2ph_pbf8(__A, __B), (__v16qi)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtne2ph_pbf8(__mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtne2ph_pbf8(__A, __B),
      (__v16qi)(__m128i)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtne2ph_pbf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2bf8_256((__v16hf)(__A),
                                                   (__v16hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtne2ph_pbf8(
    __m256i __W, __mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtne2ph_pbf8(__A, __B), (__v32qi)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtne2ph_pbf8(__mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtne2ph_pbf8(__A, __B),
      (__v32qi)(__m256i)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtnes2ph_pbf8(__m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2bf8s_128((__v8hf)(__A),
                                                    (__v8hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtnes2ph_pbf8(__m128i __W, __mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtnes2ph_pbf8(__A, __B), (__v16qi)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnes2ph_pbf8(__mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtnes2ph_pbf8(__A, __B),
      (__v16qi)(__m128i)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtnes2ph_pbf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2bf8s_256((__v16hf)(__A),
                                                    (__v16hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtnes2ph_pbf8(
    __m256i __W, __mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtnes2ph_pbf8(__A, __B), (__v32qi)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnes2ph_pbf8(__mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtnes2ph_pbf8(__A, __B),
      (__v32qi)(__m256i)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_cvtne2ph_phf8(__m128h __A,
                                                                  __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2hf8_128((__v8hf)(__A),
                                                   (__v8hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtne2ph_phf8(__m128i __W, __mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtne2ph_phf8(__A, __B), (__v16qi)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtne2ph_phf8(__mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtne2ph_phf8(__A, __B),
      (__v16qi)(__m128i)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtne2ph_phf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2hf8_256((__v16hf)(__A),
                                                   (__v16hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtne2ph_phf8(
    __m256i __W, __mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtne2ph_phf8(__A, __B), (__v32qi)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtne2ph_phf8(__mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtne2ph_phf8(__A, __B),
      (__v32qi)(__m256i)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtnes2ph_phf8(__m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2hf8s_128((__v8hf)(__A),
                                                    (__v8hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtnes2ph_phf8(__m128i __W, __mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtnes2ph_phf8(__A, __B), (__v16qi)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnes2ph_phf8(__mmask16 __U, __m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_selectb_128(
      (__mmask16)__U, (__v16qi)_mm_cvtnes2ph_phf8(__A, __B),
      (__v16qi)(__m128i)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtnes2ph_phf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2hf8s_256((__v16hf)(__A),
                                                    (__v16hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256 _mm256_mask_cvtnes2ph_phf8(
    __m256i __W, __mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtnes2ph_phf8(__A, __B), (__v32qi)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnes2ph_phf8(__mmask32 __U, __m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_selectb_256(
      (__mmask16)__U, (__v32qi)_mm256_cvtnes2ph_phf8(__A, __B),
      (__v32qi)(__m256i)_mm256_setzero_si256());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128 _mm_cvtnehf8_ph(__m128i __A) {
  return (__m128h)__builtin_ia32_vcvthf8_2ph128_mask(
      (__v16qi)__A, (__v8hf)(__m128h)_mm_undefined_ph(), (__mmask8)-1);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_cvtnehf8_ph(__m128h __W, __mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvthf8_2ph128_mask(
      (__v16qi)__A, (__v8hf)(__m128h)__W, (__mmask8)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnehf8_ph(__mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvthf8_2ph128_mask(
      (__v16qi)__A, (__v8hf)(__m128h)_mm_setzero_ph(), (__mmask8)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_cvtnehf8_ph(__m128i __A) {
  return (__m256h)__builtin_ia32_vcvthf8_2ph256_mask(
      (__v16qi)__A, (__v16hf)(__m256h)_mm256_undefined_ph(), (__mmask16)-1);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_cvtnehf8_ph(__m256h __W, __mmask16 __U, __m128i __A) {
  return (__m256h)__builtin_ia32_vcvthf8_2ph256_mask(
      (__v16qi)__A, (__v16hf)(__m256h)__W, (__mmask16)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnehf8_ph(__mmask16 __U, __m128i __A) {
  return (__m256h)__builtin_ia32_vcvthf8_2ph256_mask(
      (__v16qi)__A, (__v16hf)(__m256h)_mm256_setzero_ph(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_cvtneph_pbf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtneph_pbf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneph_pbf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtneph_pbf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneph_pbf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneph_pbf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_cvtnesph_pbf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtnesph_pbf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnesph_pbf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtnesph_pbf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtnesph_pbf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnesph_pbf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_cvtneph_phf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtneph_phf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneph_phf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtneph_phf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneph_phf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneph_phf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_cvtnesph_phf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtnesph_phf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnesph_phf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_128_mask(
      (__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtnesph_phf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtnesph_phf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnesph_phf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_256_mask(
      (__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128 _mm_cvtpbf8_ph(__m128i __A) {
  return _mm_castsi128_ph(_mm_slli_epi16(_mm_cvtepi8_epi16(__A), 8));
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_cvtpbf8_ph(__m128h __S, __mmask8 __U, __m128i __A) {
  return _mm_castsi128_ph(
      _mm_mask_slli_epi16((__m128i)__S, __U, _mm_cvtepi8_epi16(__A), 8));
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_cvtpbf8_ph(__mmask8 __U, __m128i __A) {
  return _mm_castsi128_ph(_mm_slli_epi16(_mm_maskz_cvtepi8_epi16(__U, __A), 8));
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256 _mm256_cvtpbf8_ph(__m128i __A) {
  return _mm256_castsi256_ph(_mm256_slli_epi16(_mm256_cvtepi8_epi16(__A), 8));
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_cvtpbf8_ph(__m256h __S, __mmask8 __U, __m128i __A) {
  return _mm256_castsi256_ph(
      _mm256_mask_slli_epi16((__m256i)__S, __U, _mm256_cvtepi8_epi16(__A), 8));
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtpbf8_ph(__mmask8 __U, __m128i __A) {
  return _mm256_castsi256_ph(
      _mm256_slli_epi16(_mm256_maskz_cvtepi8_epi16(__U, __A), 8));
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX10_2CONVERTINTRIN_H
#endif // __SSE2__
