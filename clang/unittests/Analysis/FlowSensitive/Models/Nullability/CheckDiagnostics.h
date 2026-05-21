// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_CLANG_UNITTESTS_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_CHECKDIAGNOSTICS_H
#define LLVM_CLANG_UNITTESTS_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_CHECKDIAGNOSTICS_H

#include "clang/AST/ASTContext.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/Pragma.h"
#include "clang/Testing/CommandLineArgs.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Testing/Annotations/Annotations.h"

namespace clang::dataflow::nullability {

/// Runs nullability verification on `SourceCode` and returns whether
/// diagnostics are produced on those lines marked in the source code with
/// `llvm::Annotations` style annotations (and no other lines).
/// TODO(mboehme): So far, we only check the locations of the diagnostics; it
/// would be desirable to check their actual content too.
bool checkDiagnostics(llvm::StringRef SourceCode);

/// Same as `checkDiagnostics`, but allows for untracked errors.
bool checkDiagnosticsHasUntracked(llvm::StringRef SourceCode);

bool checkDiagnosticsWithMin(llvm::StringRef SourceCode, TestLanguage Min);

bool checkDiagnostics(ASTContext &AST, llvm::Annotations AnnotatedCode,
                      const NullabilityPragmas &Pragmas = NullabilityPragmas(),
                      bool AllowUntracked = false);

} // namespace clang::dataflow::nullability

#endif // LLVM_CLANG_UNITTESTS_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_CHECKDIAGNOSTICS_H
