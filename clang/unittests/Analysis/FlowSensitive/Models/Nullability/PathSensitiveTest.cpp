// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Tests that involve path-sensitivity.

#include "CheckDiagnostics.h"
#include "gtest/gtest.h"

namespace clang::dataflow::nullability {
namespace {

TEST(PointerNullabilityTest, ConditionalInitialization) {
  // `SimpleBoolModel` creates unconstrained atoms when joining `p` after
  // `if (!b) p = produce_int()`, without recording that `!b` implies `p` is
  // non-null. Consequently, after `if (b) p = produce_int()`, the `!b` branch
  // still considers `p` nullable.
  EXPECT_TRUE(checkDiagnostics(R"cc(
    struct S {
      bool b;
    };
    S return_s();
    int* _Nonnull produce_int();

    void target(int i) {
      int* p = nullptr;
      bool b = false;
      if (i < 0) {
        S s = return_s();
        b = s.b;
        if (!b) p = produce_int();
      } else {
        S s = return_s();
        b = s.b;
        if (!b) p = produce_int();
      }
      if (b) p = produce_int();

      (void)*p;  $unsafe_if_simple
    }
  )cc"));
}

TEST(PointerNullabilityTest, ConditionalInitialization2) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    bool return_bool();
    int *_Nonnull produce_int();

    void target() {
      int *p = nullptr;
      bool b = false;
      b = return_bool();
      if (!b) p = produce_int();

      (void)*p;  // [[unsafe]]
    }
  )cc"));
}

TEST(PointerNullabilityTest, ComplexLoopCondition) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    int* _Nullable produce_int();
    void target() {
      int* p1;
      int* p2;
      while ((p1 = produce_int()) != nullptr && (p2 = produce_int()) != nullptr) {
        *p1;
        *p2;
      }
    }
  )cc"));
}

TEST(PointerNullabilityTest, OnePointerGuaranteedNonnull) {
  // On the RHS fallthrough edge of short-circuit `if (!p1 && !p2) return;`,
  // `SimpleBoolModel::assume` reduces `!(!p1 && !p2)` immediately using the
  // branch-local `p1 == nullptr` assignment and does not retain the disjunctive
  // formula in `AssumedFormulas` across the join with the LHS short-circuit
  // edge, so it cannot deduce that `p2` is non-null in the `else` branch.
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target(int *_Nullable p1, int *_Nullable p2) {
      if (!p1 && !p2) return;
      if (p1)
        *p1;
      else
        // `p2` must be nonnull or we would have returned above.
        *p2;  $unsafe_if_simple
    }
  )cc"));
}

// The check cannot reason about integer inequalities, so does not know that the
// loop has at least one iteration.
TEST(PointerNullabilityTest, DereferencePointerSetInLoop) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target() {
      int* _Nullable p = nullptr;
      int x = 0;
      for (int i = 0; i < 10; ++i) {
        p = &x;
      }
      *p = 1;  // [[unsafe]]
    }
  )cc"));
}

// A do-while loop makes it clear there's at least one iteration.
TEST(PointerNullabilityTest, DereferencePointerSetInDoWhileLoop) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target() {
      int* _Nullable p = nullptr;
      int x = 0;
      int i = 0;
      do {
        p = &x;
        ++i;
      } while (i < 10);
      *p = 1;
    }
  )cc"));
}

} // namespace
} // namespace clang::dataflow::nullability
