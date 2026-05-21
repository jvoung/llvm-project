// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Tests for nullability with variable aliasing.

#include "CheckDiagnostics.h"
#include "gtest/gtest.h"

namespace clang::dataflow::nullability {
namespace {

TEST(PointerNullabilityTest, NullCheckAliasDereferenceOriginal) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target(int* _Nullable x) {
      int* y = x;
      *x;  // [[unsafe]]
      if (y) {
        *x;
      } else {
        *x;  // [[unsafe]]
      }
      *x;  // [[unsafe]]
    }
  )cc"));
}

TEST(PointerNullabilityTest, NullCheckOriginalDereferenceAlias) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target(int* _Nullable x) {
      int* y = x;
      *y;  // [[unsafe]]
      if (x) {
        *y;
      } else {
        *y;  // [[unsafe]]
      }
      *y;  // [[unsafe]]
    }
  )cc"));
}

} // namespace
} // namespace clang::dataflow::nullability
