// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Tests for nullability correctness of default arguments.

#include "CheckDiagnostics.h"
#include "gtest/gtest.h"

namespace clang::dataflow::nullability {
namespace {

TEST(PointerNullabilityTest, DefaultArgNonnull) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target(int *_Nonnull = nullptr /* [[unsafe]] */);
  )cc"));
}

TEST(PointerNullabilityTest, DefaultArgNullable) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target(int *_Nullable = nullptr);
  )cc"));
}

TEST(PointerNullabilityTest, DefaultArgUnannotated) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    void target(int* = nullptr);
  )cc"));
}

TEST(PointerNullabilityTest, DefaultArgNonnullValueNullable) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    int *_Nullable p;
    void target(int *_Nonnull = p /* [[unsafe]] */);
  )cc"));
}

TEST(PointerNullabilityTest, DefaultArgNonnullValueNonnull) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    int *_Nonnull p;
    void target(int *_Nonnull = p);
  )cc"));
}

TEST(PointerNullabilityTest, DefaultArgNonnullValueUnannotated) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    int* p;
    void target(int* _Nonnull = p);
  )cc"));
}

} // namespace
} // namespace clang::dataflow::nullability
