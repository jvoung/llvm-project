// RUN: %clang_cc1 -verify -fopenmp %s -triple x86_64-apple-darwin10 -x c++ -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fopenmp -x c++ -std=c++11 -emit-pch -o %t %s -triple x86_64-apple-darwin10
// RUN: %clang_cc1 -fopenmp -std=c++11 -include-pch %t -verify %s -triple x86_64-apple-darwin10 -x c++ -emit-llvm -o -| FileCheck %s

// RUN: %clang_cc1 -verify -fopenmp-simd %s -x c++ -emit-llvm -triple x86_64-apple-darwin10 -o -| FileCheck %s --check-prefix SIMD-ONLY0
// RUN: %clang_cc1 -fopenmp-simd -x c++ -std=c++11 -emit-pch -o %t %s -triple x86_64-apple-darwin10
// RUN: %clang_cc1 -fopenmp-simd -std=c++11 -include-pch %t -verify %s -x c++ -emit-llvm -triple x86_64-apple-darwin10 -o -| FileCheck %s --check-prefix SIMD-ONLY0
// SIMD-ONLY0-NOT: {{__kmpc|__tgt}}
// expected-no-diagnostics

#ifndef HEADER
#define HEADER

#pragma omp requires atomic_default_mem_order(acq_rel)

// CHECK-LABEL: foo
void foo() {
  int a = 0, b = 0;
// CHECK: load atomic i32, {{.*}} acquire, align 4
#pragma omp atomic read
  a = b;
// CHECK: store atomic i32 {{.*}} release, align 4
#pragma omp atomic write
  a = b;
// CHECK: atomicrmw add ptr {{.*}} release, align 4
#pragma omp atomic
  a += 1;
// CHECK: atomicrmw add ptr {{.*}} release, align 4
#pragma omp atomic update
  a += 1;
// CHECK: atomicrmw add ptr {{.*}} acq_rel, align 4
#pragma omp atomic capture
  {
    b = a;
    a += 1;
  }
}

#endif
