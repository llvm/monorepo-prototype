// RUN: mlir-query %s -c "m definedBy(hasOpName(())" | FileCheck %s

func.func @region_control_flow(%arg: memref<2xf32>, %cond: i1) attributes {test.ptr = "func"} {
  %0 = memref.alloca() {test.ptr = "alloca_1"} : memref<8x64xf32>
  %1 = memref.alloca() {test.ptr = "alloca_2"} : memref<8x64xf32>
  %2 = memref.alloc() {test.ptr = "alloc_1"} : memref<8x64xf32>

  %3 = scf.if %cond -> (memref<8x64xf32>) {
    scf.yield %0 : memref<8x64xf32>
  } else {
    scf.yield %0 : memref<8x64xf32>
  } {test.ptr = "if_alloca"}

  %4 = scf.if %cond -> (memref<8x64xf32>) {
    scf.yield %0 : memref<8x64xf32>
  } else {
    scf.yield %1 : memref<8x64xf32>
  } {test.ptr = "if_alloca_merge"}

  %5 = scf.if %cond -> (memref<8x64xf32>) {
    scf.yield %2 : memref<8x64xf32>
  } else {
    scf.yield %2 : memref<8x64xf32>
  } {test.ptr = "if_alloc"}
  return
}
