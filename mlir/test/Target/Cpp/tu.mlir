// RUN: mlir-translate -mlir-to-cpp %s | FileCheck %s --check-prefix NO-FILTER
// RUN: mlir-translate -mlir-to-cpp -translation-unit-id=non-existing %s | FileCheck %s --check-prefix NON-EXISTING
// RUN: mlir-translate -mlir-to-cpp -translation-unit-id=tu_one %s | FileCheck %s --check-prefix TU-ONE
// RUN: mlir-translate -mlir-to-cpp -translation-unit-id=tu_two %s | FileCheck %s --check-prefix TU-TWO


// NO-FILTER-NOT: func_one
// NO-FILTER-NOT: func_two

// NON-EXISTING-NOT: func_one
// NON-EXISTING-NOT: func_two

// TU-ONE: func_one
// TU-ONE-NOT: func_two

// TU-TWO-NOT: func_one
// TU-TWO: func_two

emitc.tu "tu_one" {
  emitc.func @func_one(%arg: f32) {
    emitc.return
  }
}

emitc.tu "tu_two" {
  emitc.func @func_two(%arg: f32) {
    emitc.return
  }
}
