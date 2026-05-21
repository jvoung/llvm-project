// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Tests for `checkDiagnostics()` itself.

#include "CheckDiagnostics.h"

#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

namespace clang::dataflow::nullability {
namespace {

TEST(PointerNullabilityTest, NoDiagnostics) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target() {}
  )cc"));
}

TEST(PointerNullabilityTest, ExpectedDiagnostic) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target() {
      int *p = nullptr;
      *p;  // [[unsafe]]
    }
  )cc"));
}

TEST(PointerNullabilityTest, UnexpectedDiagnostic) {
  bool Result = true;
  EXPECT_NONFATAL_FAILURE(Result = checkDiagnostics(R"cc(
                            void target() {
                              1;  // [[unsafe]]
                            }
                          )cc"),
                          "Expected diagnostics but didn't find them");
  EXPECT_EQ(Result, false);
}

TEST(PointerNullabilityTest, MissingDiagnostic) {
  bool Result = true;
  EXPECT_NONFATAL_FAILURE(Result = checkDiagnostics(R"cc(
                            void target() {
                              int *p = nullptr;
                              *p;  // Missing diagnostic
                            }
                          )cc"),
                          "Found diagnostics but didn't expect them");
  EXPECT_EQ(Result, false);
}

} // namespace
} // namespace clang::dataflow::nullability
