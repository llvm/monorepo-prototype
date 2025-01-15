//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___FWD_BIT_REFERENCE_H
#define _LIBCPP___FWD_BIT_REFERENCE_H

#include <__assert>
#include <__config>
#include <__type_traits/enable_if.h>
#include <__type_traits/is_unsigned.h>
#include <climits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Cp, bool _IsConst, typename _Cp::__storage_type = 0>
class __bit_iterator;

template <class, class = void>
struct __size_difference_type_traits;

template <class _StorageType, __enable_if_t<is_unsigned<_StorageType>::value, int> = 0>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR _StorageType
__fill_range_in_word(_StorageType __word, unsigned __ctz, unsigned __clz, bool __fill_val) {
  _LIBCPP_ASSERT_VALID_INPUT_RANGE(
      __ctz + __clz < sizeof(_StorageType) * CHAR_BIT, "__fill_range called with invalid range");
  // When _StorageType is some smaller integral type like `uint8_t`, `uint16_t`, or `unsigned short`, casting
  // back to _StorageType is necessary to avoid UB due to integral promotions.
  // See https://github.com/llvm/llvm-project/pull/122410
  _StorageType __m = static_cast<_StorageType>(static_cast<_StorageType>(~static_cast<_StorageType>(0)) << __ctz) &
                     static_cast<_StorageType>(static_cast<_StorageType>(~static_cast<_StorageType>(0)) >> __clz);
  return __fill_val ? __word | __m : __word & __m;
}

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___FWD_BIT_REFERENCE_H
