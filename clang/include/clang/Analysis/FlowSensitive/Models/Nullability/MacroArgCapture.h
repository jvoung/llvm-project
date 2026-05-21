// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// This file provides shared constants used when capturing the values passed as
// arguments to macros of interest.

#ifndef LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_MACROARGCAPTURE_H
#define LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_MACROARGCAPTURE_H

#include "llvm/ADT/StringRef.h"

namespace clang::dataflow::nullability {
inline constexpr llvm::StringRef ArgCaptureAbortIfFalse =
    "clang_tidy_nullability_internal_abortIfFalse";

inline constexpr llvm::StringRef ArgCaptureAbortIfEqual =
    "clang_tidy_nullability_internal_abortIfEqual";
} // namespace clang::dataflow::nullability

#endif // LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_MACROARGCAPTURE_H
