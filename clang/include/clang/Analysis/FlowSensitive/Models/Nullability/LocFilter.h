// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_LOCFILTER_H
#define LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_LOCFILTER_H

#include <memory>

#include "clang/AST/DeclBase.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

namespace clang::dataflow::nullability {

// An interface for filtering SourceLocations.
class LocFilter {
public:
  virtual ~LocFilter() = default;
  virtual bool check(SourceLocation Loc) = 0;
};

enum class LocFilterKind {
  kAllowAll,         // No filtering.
  kMainFileOrHeader, // Restrict to the main file or its associated header.
  kAllowAllButNotMainFile,   // Restrict to all files but the main file.
  kMainHeaderButNotMainFile, // Restrict to the header associated with the
                             // main file, but not the main file itself.
};

// Returns a LocFilter that restricts according to the given LocFilterKind.
std::unique_ptr<LocFilter> getLocFilter(const SourceManager &SM,
                                        LocFilterKind Kind);

} // namespace clang::dataflow::nullability

#endif // LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_LOCFILTER_H
