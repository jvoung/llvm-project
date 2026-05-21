//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_POINTERNULLABILITYCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_POINTERNULLABILITYCHECK_H

#include <memory>

#include "../ClangTidyCheck.h"
#include "../ClangTidyDiagnosticConsumer.h"
#include "../ClangTidyOptions.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/LocFilter.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/Pragma.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/LangOptions.h"

namespace clang::tidy::bugprone {

/// Warns when the code is does not adhere to the contract of the Clang
/// Nullability annotations, to prevent runtime dereferences of `nullptr`.
///
/// For the user-facing documentation see: TODO
class PointerNullabilityCheck : public ClangTidyCheck {
public:
  PointerNullabilityCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context),
        CheckMainFileOnly(Options.get("MainFileOnly", true)) {}

  void registerPPCallbacks(const SourceManager &, Preprocessor *,
                           Preprocessor *ModuleExpanderPP) override;

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

  void onEndOfTranslationUnit() override;

  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override {
    Options.store(Opts, "MainFileOnly", CheckMainFileOnly);
  }

private:
  void initializeLocFilter(const ast_matchers::MatchFinder::MatchResult &Result,
                           SourceManager &SM);

  // When set, only checks the main file and its associated headers, skipping
  // headers that are only transitively included. This is useful if the UI
  // wouldn't show diagnostics in other files anyway (e.g., for code review).
  bool CheckMainFileOnly;

  // Source location filter checking whether a location is in the main file or a
  // header associated with the main file, i.e. a header that has the same file
  // path except for the extension.
  std::unique_ptr<clang::dataflow::nullability::LocFilter>
      InMainFileOrHeaderFilter;

  // During preprocessing, capture per-file nullability pragmas.
  clang::dataflow::nullability::NullabilityPragmas Pragmas;
};

} // namespace clang::tidy::bugprone

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_POINTERNULLABILITYCHECK_H
