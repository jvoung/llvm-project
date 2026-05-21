// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// This file defines mock headers for use in nullability tests.

#ifndef THIRD_PARTY_LLVM_CLANG_UNITTESTS_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_MOCKHEADERS_H
#define THIRD_PARTY_LLVM_CLANG_UNITTESTS_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_MOCKHEADERS_H

#include <utility>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace clang::dataflow::nullability::test {

llvm::ArrayRef<std::pair<llvm::StringRef, llvm::StringRef>> getMockHeaders();

} // namespace clang::dataflow::nullability::test

#endif // THIRD_PARTY_LLVM_CLANG_UNITTESTS_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_MOCKHEADERS_H
