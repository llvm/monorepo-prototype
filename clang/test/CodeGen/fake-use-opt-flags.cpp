/// Check that we generate fake uses only when -fextend-lifetimes is set and we
/// are not setting optnone, or when we have optimizations set to -Og and we have
/// not passed -fno-extend-lifetimes.
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O0 -fextend-lifetimes %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O0 -disable-O0-optnone -fextend-lifetimes %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -Og %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -Og -fno-extend-lifetimes %s -o - | FileCheck %s

/// Test various optimization flags with -fextend-lifetimes...
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O1 %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O1 -fextend-lifetimes %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O2 %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O2 -fextend-lifetimes %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O3 %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -O3 -fextend-lifetimes %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -Os %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -Os -fextend-lifetimes %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -Oz %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -disable-llvm-passes -Oz -fextend-lifetimes %s -o - | FileCheck %s --check-prefixes=CHECK,EXTEND

// CHECK-LABEL: define{{.*}} void @_Z3fooi(i32{{.*}} %a)
// CHECK-NEXT:  entry:
// CHECK-NEXT:    %a.addr = alloca i32
// CHECK-NEXT:    store i32 %a, ptr %a.addr
// EXTEND-NEXT:   %fake.use = load i32, ptr %a.addr
// EXTEND-NEXT:   call void (...) @llvm.fake.use(i32 %fake.use)
// CHECK-NEXT:    ret void
// CHECK-NEXT:  }

void foo(int a) {}
