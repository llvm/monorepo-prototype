// REQUIRES: x86
// RUN: split-file %s %t.dir
// RUN: llvm-lib -machine:amd64 -out:%t.lib -def:%t.dir/lib.def
// RUN: llvm-mc -filetype=obj -triple=x86_64-windows %t.dir/main.s -o %t.main.obj

// RUN: yaml2obj %S/Inputs/msvc-implib1.yaml -o %t.msvc-implib1.obj
// RUN: yaml2obj %S/Inputs/msvc-implib2.yaml -o %t.msvc-implib2.obj
// RUN: yaml2obj %S/Inputs/msvc-implib3.yaml -o %t.msvc-implib3.obj
// RUN: llvm-lib -out:%t-msvc.lib %t.msvc-implib1.obj %t.msvc-implib2.obj %t.msvc-implib3.obj
// RUN: llvm-mc -filetype=obj -triple=x86_64-windows %t.dir/main-nocall.s -o %t.main-nocall.obj

// RUN: lld-link -out:%t.exe %t.main.obj -wholearchive:%t.lib -entry:entry -subsystem:console
// RUN: llvm-readobj --coff-imports %t.exe | FileCheck %s

// RUN: lld-link -out:%t-msvc.exe %t.main-nocall.obj -wholearchive:%t-msvc.lib -entry:entry -subsystem:console
// RUN: llvm-readobj --coff-imports %t-msvc.exe | FileCheck %s --check-prefix=CHECK-MSVC

// As LLD usually doesn't use the header/trailer object files from import
// libraries, but instead synthesizes those structures, we end up with two
// import directory entries if we force those objects to be included.

// CHECK:      Import {
// CHECK-NEXT:   Name: lib.dll
// CHECK-NEXT:   ImportLookupTableRVA: 0x2050
// CHECK-NEXT:   ImportAddressTableRVA: 0x2068
// CHECK-NEXT: }
// CHECK-NEXT: Import {
// CHECK-NEXT:   Name: lib.dll
// CHECK-NEXT:   ImportLookupTableRVA: 0x2058
// CHECK-NEXT:   ImportAddressTableRVA: 0x2070
// CHECK-NEXT:   Symbol: func (0)
// CHECK-NEXT: }

// CHECK-MSVC:      Import {
// CHECK-MSVC-NEXT:   Name: foo.dll
// CHECK-MSVC-NEXT:   ImportLookupTableRVA: 0x203C
// CHECK-MSVC-NEXT:   ImportAddressTableRVA: 0x2048
// CHECK-MSVC-NEXT: }


#--- main.s
.global entry
entry:
  call func
  ret

#--- main-nocall.s
.global entry
entry:
  ret

#--- lib.def
LIBRARY lib.dll
EXPORTS
func
