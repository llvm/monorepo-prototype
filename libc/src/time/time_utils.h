//===-- Collection of utils for mktime and friends --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIBC_SRC_TIME_TIME_UTILS_H
#define LLVM_LIBC_SRC_TIME_TIME_UTILS_H

#include <stddef.h> // For size_t.
#include "src/__support/CPP/limits.h"
#include "src/__support/common.h"
#include "src/__support/macros/config.h"
#include "src/errno/libc_errno.h"
#include "time_constants.h"
#include "src/time/mktime.h"

#include <stdint.h>

namespace LIBC_NAMESPACE_DECL {
namespace time_utils {

// Update the "tm" structure's year, month, etc. members from seconds.
// "total_seconds" is the number of seconds since January 1st, 1970.
extern int64_t update_from_seconds(int64_t total_seconds, struct tm *tm);
extern int calculate_dst(struct tm *tm);

// TODO(michaelrj): move these functions to use ErrorOr instead of setting
// errno. They always accompany a specific return value so we only need the one
// variable.

// POSIX.1-2017 requires this.
LIBC_INLINE time_t out_of_range() {
#ifdef EOVERFLOW
  // For non-POSIX uses of the standard C time functions, where EOVERFLOW is
  // not defined, it's OK not to set errno at all. The plain C standard doesn't
  // require it.
  libc_errno = EOVERFLOW;
#endif
  return time_constants::OUT_OF_RANGE_RETURN_VALUE;
}

LIBC_INLINE void invalid_value() { libc_errno = EINVAL; }

LIBC_INLINE char *asctime(const struct tm *timeptr, char *buffer,
                          size_t bufferLength) {
  if (timeptr == nullptr || buffer == nullptr) {
    invalid_value();
    return nullptr;
  }
  if (timeptr->tm_wday < 0 ||
      timeptr->tm_wday > (time_constants::DAYS_PER_WEEK - 1)) {
    invalid_value();
    return nullptr;
  }
  if (timeptr->tm_mon < 0 ||
      timeptr->tm_mon > (time_constants::MONTHS_PER_YEAR - 1)) {
    invalid_value();
    return nullptr;
  }

  // TODO(michaelr): move this to use the strftime machinery
  int written_size = __builtin_snprintf(
      buffer, bufferLength, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
      time_constants::WEEK_DAY_NAMES[timeptr->tm_wday].data(),
      time_constants::MONTH_NAMES[timeptr->tm_mon].data(), timeptr->tm_mday,
      timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec,
      time_constants::TIME_YEAR_BASE + timeptr->tm_year);
  if (written_size < 0)
    return nullptr;
  if (static_cast<size_t>(written_size) >= bufferLength) {
    out_of_range();
    return nullptr;
  }
  return buffer;
}

LIBC_INLINE struct tm *gmtime_internal(const time_t *timer, struct tm *result) {
  int64_t seconds = *timer;
  // Update the tm structure's year, month, day, etc. from seconds.
  if (update_from_seconds(seconds, result) < 0) {
    out_of_range();
    return nullptr;
  }

  return result;
}

LIBC_INLINE struct tm *localtime(const time_t *t_ptr) {
  static struct tm result;
  int64_t time = *t_ptr;

  // Update the tm structure's year, month, day, etc. from seconds.
  if (update_from_seconds(time, &result) < 0) {
    out_of_range();
    return nullptr;
  }

  int isdst = calculate_dst(&result);
  result.tm_isdst = isdst;

  return &result;
}

LIBC_INLINE struct tm *localtime_internal(const time_t *t_ptr,
                                          struct tm *input) {
  static struct tm result;
  int64_t t = *t_ptr;

  result.tm_sec = input->tm_sec;
  result.tm_min = input->tm_min;
  result.tm_hour = input->tm_hour;
  result.tm_mday = input->tm_mday;
  result.tm_mon = input->tm_mon;
  result.tm_year = input->tm_year;
  result.tm_wday = input->tm_wday;
  result.tm_yday = input->tm_yday;
  result.tm_isdst = input->tm_isdst;

  // Update the tm structure's year, month, day, etc. from seconds.
  if (update_from_seconds(t, &result) < 0) {
    out_of_range();
    return nullptr;
  }

  int isdst = calculate_dst(&result);
  result.tm_isdst = isdst;

  return &result;
}

// for windows only, implemented on gnu/linux for compatibility reasons
LIBC_INLINE int localtime_s(const time_t *t_ptr, struct tm *input) {
  static struct tm *result = localtime_internal(t_ptr, input);
  time_t t = LIBC_NAMESPACE::mktime(result);

  if (*t_ptr < t) {
    return -1;
  }

  if (*t_ptr > t) {
    return 1;
  }

  return 0;
}

} // namespace time_utils
} // namespace LIBC_NAMESPACE_DECL

#endif // LLVM_LIBC_SRC_TIME_TIME_UTILS_H
