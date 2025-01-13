//===-- Unittests for strftime --------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "hdr/types/struct_tm.h"
#include "src/time/strftime.h"
#include "test/UnitTest/Test.h"

// Copied from sprintf_test.cpp.
// TODO: put this somewhere more reusable, it's handy.
// Subtract 1 from sizeof(expected_str) to account for the null byte.
#define EXPECT_STREQ_LEN(actual_written, actual_str, expected_str)             \
  EXPECT_EQ(actual_written, sizeof(expected_str) - 1);                         \
  EXPECT_STREQ(actual_str, expected_str);

TEST(LlvmLibcStrftimeTest, FullYearTests) {
  // this tests %Y, which reads: [tm_year]
  struct tm time;
  char buffer[100];
  size_t written = 0;

  // basic tests
  time.tm_year = 2022 - 1900; // tm_year counts years since 1900, so 122 -> 2022
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "2022");

  time.tm_year = 11900 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "11900");

  time.tm_year = 1900 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "1900");

  time.tm_year = 900 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "900");

  time.tm_year = 0 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0");

  time.tm_year = -1 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-1");

  time.tm_year = -9001 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-9001");

  time.tm_year = -10001 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-10001");

  // width tests (with the 0 flag, since the default padding is undefined).
  time.tm_year = 2023 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%01Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "2023");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%04Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "2023");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%05Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "02023");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%010Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0000002023");

  time.tm_year = 900 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%01Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "900");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%04Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0900");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%05Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "00900");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%010Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0000000900");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%01Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "12345");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%04Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "12345");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%05Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "12345");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%010Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0000012345");

  time.tm_year = -123 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%01Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-123");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%04Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-123");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%05Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-0123");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%010Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-000000123");

  // '+' flag tests
  time.tm_year = 2023 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+1Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "2023");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "2023");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+5Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+2023");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+10Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+000002023");

  time.tm_year = 900 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+1Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "900");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0900");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+5Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+0900");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+10Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+000000900");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+1Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+12345");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+12345");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+5Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+12345");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+10Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+000012345");

  time.tm_year = -123 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+1Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-123");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-123");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+5Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-0123");

  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+10Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "-000000123");

  // Posix specified tests:
  time.tm_year = 1970 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "1970");

  time.tm_year = 1970 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "1970");

  time.tm_year = 27 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "27");

  time.tm_year = 270 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "270");

  time.tm_year = 270 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "0270");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "12345");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+4Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+12345");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%05Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "12345");

  time.tm_year = 270 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+5Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+0270");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+5Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+12345");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%06Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "012345");

  time.tm_year = 12345 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+6Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+12345");

  time.tm_year = 123456 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%08Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "00123456");

  time.tm_year = 123456 - 1900;
  written = LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%+8Y", &time);
  EXPECT_STREQ_LEN(written, buffer, "+0123456");
}

// TODO: tests for each other conversion.

TEST(LlvmLibcStrftimeTest, CompositeTests) {
  struct tm time;
  time.tm_year = 122; // Year since 1900, so 2022
  time.tm_mon = 9;    // October (0-indexed)
  time.tm_mday = 15;  // 15th day

  char buffer[100];
  LIBC_NAMESPACE::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &time);
  EXPECT_STREQ(buffer, "2022-10-15");
}
