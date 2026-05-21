// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Tests for the treatment of the `this` pointer (which is always nonnull).
#include <string>

#include "CheckDiagnostics.h"
#include "gtest/gtest.h"

namespace clang::dataflow::nullability {
namespace {

TEST(PointerNullabilityTest, ImplicitThis) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    struct Foo {
      void foo();
      void target() {
        __assert_nullability<NK_nonnull>(this);
        foo();
      }
    };
  )cc"));
}

TEST(PointerNullabilityTest, ExplicitThis) {
  // (->) explicit `this`
  EXPECT_TRUE(checkDiagnostics(R"cc(
    struct Foo {
      void foo();
      void target() { this->foo(); }
    };
  )cc"));
}

TEST(PointerNullabilityTest, ClassWithPointerTemplateArg) {
  EXPECT_TRUE(checkDiagnostics(R"cc(
    template <class T>
    struct S;
    template <>
    struct S<int *_Nullable> {
      void target() {
        // `_Nullable` in the specialization is bogus: we can't specialize on
        // nullability as it's just sugar. Therefore the correct inner
        // nullability here is "unspecified".
        __assert_nullability<NK_nonnull, NK_unspecified>(this);
      }
    };
  )cc"));
}

} // namespace
} // namespace clang::dataflow::nullability
