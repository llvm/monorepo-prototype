// RUN: mlir-query %s -c "m definedBy(hasOpName(())" | FileCheck %s


func.func @matrix_multiply(%A: memref<4x4xf32>, %B: memref<4x4xf32>, %C: memref<4x4xf32>) {
  %c0 = arith.constant 0 : index
  %c4 = arith.constant 4 : index
  %c1 = arith.constant 1 : index

  scf.for %i = %c0 to %c4 step %c1 {
    scf.for %j = %c0 to %c4 step %c1 {
      %sum_init = arith.constant 0.0 : f32
      %sum = scf.for %k = %c0 to %c4 step %c1 iter_args(%acc = %sum_init) -> (f32) {
        %a_ik = memref.load %A[%i, %k] : memref<4x4xf32>
        %b_kj = memref.load %B[%k, %j] : memref<4x4xf32>
        %prod = arith.mulf %a_ik, %b_kj : f32
        %new_acc = arith.addf %acc, %prod : f32
        scf.yield %new_acc : f32
      }
      memref.store %sum, %C[%i, %j] : memref<4x4xf32>
    }
  }
  return
}