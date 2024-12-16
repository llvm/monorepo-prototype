//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "benchmark/benchmark.h"
#include "ContainerBenchmarks.h"
#include "../GenerateInput.h"

using namespace ContainerBenchmarks;

BENCHMARK_CAPTURE(BM_ConstructIterIter, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)->Arg(5140480);
BENCHMARK_CAPTURE(BM_ConstructFromRange, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)->Arg(5140480);

BENCHMARK_CAPTURE(BM_assign_iter_iter, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)->Arg(5140480);
BENCHMARK_CAPTURE(BM_assign_range, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)->Arg(5140480);

BENCHMARK_CAPTURE(BM_insert_iter_iter_iter, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)
    ->Arg(5140480);
BENCHMARK_CAPTURE(BM_insert_range, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)->Arg(5140480);
BENCHMARK_CAPTURE(BM_append_range, vector_bool, std::vector<bool>{}, getRandomIntegerInputs<bool>)->Arg(5140480);

BENCHMARK_MAIN();
