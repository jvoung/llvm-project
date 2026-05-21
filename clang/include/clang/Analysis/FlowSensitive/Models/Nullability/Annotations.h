// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_ANNOTATIONS_H
#define THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_ANNOTATIONS_H

#include "llvm/ADT/StringRef.h"

namespace clang::dataflow::nullability {
inline constexpr llvm::StringLiteral ClangNullable = "_Nullable";
inline constexpr llvm::StringLiteral ClangNonnull = "_Nonnull";
inline constexpr llvm::StringLiteral ClangUnknown = "_Null_unspecified";

inline constexpr llvm::StringLiteral AbslMacroNullable = "";
inline constexpr llvm::StringLiteral AbslMacroNonnull = "";
inline constexpr llvm::StringLiteral AbslMacroUnknown =
    "absl_nullability_unknown";
inline constexpr llvm::StringLiteral AbslMacroConflict =
    "absl_nullability_conflict";
} // namespace clang::dataflow::nullability

#endif // THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_ANNOTATIONS_H
