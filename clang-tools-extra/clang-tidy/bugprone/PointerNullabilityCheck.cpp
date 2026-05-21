//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "PointerNullabilityCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/LocFilter.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/PointerNullabilityDiagnosis.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/Pragma.h"
#include "clang/Analysis/FlowSensitive/Solver.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
namespace clang::tidy::bugprone {

using ast_matchers::MatchFinder;
using clang::dataflow::nullability::PointerNullabilityDiagnostic;
using llvm::StringLiteral;

static bool shouldReportError(PointerNullabilityDiagnostic::ErrorCode EC) {
  return EC != PointerNullabilityDiagnostic::ErrorCode::Untracked;
}

static llvm::StringRef
diagnosticExpectedNonNullMessage(PointerNullabilityDiagnostic::Context Ctx) {
  switch (Ctx) {
  case PointerNullabilityDiagnostic::Context::Initializer:
    return "initializing a Nonnull annotated variable with a nullable "
           "value";
  case PointerNullabilityDiagnostic::Context::Assignment:
    return "assigning a nullable value to a Nonnull annotated pointer";
  case PointerNullabilityDiagnostic::Context::NullableDereference:
    return "dereferencing a potentially nullable pointer";
  case PointerNullabilityDiagnostic::Context::ReturnValue:
    return "returning a nullable pointer in a function that has a Nonnull "
           "return type";
  case PointerNullabilityDiagnostic::Context::FunctionArgument:
    return "expected %select{unnamed parameter|parameter %1}0"
           "%select{| of %q3}2 to be nonnull, but a nullable argument was "
           "used";
  case PointerNullabilityDiagnostic::Context::Other:
    return "using a nullable pointer where a non-null pointer is expected";
  }
}

static llvm::StringRef
diagnosticExpectedEqualMessage(PointerNullabilityDiagnostic::Context Ctx) {
  switch (Ctx) {
  case PointerNullabilityDiagnostic::Context::Initializer:
    return "initializing a variable with a value expected to have identical "
           "nullability";
  case PointerNullabilityDiagnostic::Context::Assignment:
    return "assigning a value to a type expected to have identical "
           "nullability";
  case PointerNullabilityDiagnostic::Context::NullableDereference: {
    assert(false);
    return "unexpected diagnostic context";
  }
  case PointerNullabilityDiagnostic::Context::ReturnValue:
    return "returning a value expected to have identical nullability to the "
           "return type";
  case PointerNullabilityDiagnostic::Context::FunctionArgument:
    return "expected %select{unnamed parameter|parameter %1}0"
           "%select{| of %q3}2 to have identical nullability to the argument "
           "used";
  case PointerNullabilityDiagnostic::Context::Other:
    return "pointer types were expected to have identical nullability, such "
           "as in a mutable context or in a template argument";
  }
}

/// Returns a message for the given diagnostic.
/// If the diagnostic is for a function argument, expects an argument for the
/// parameter name.
static llvm::StringRef diagnosticMessage(PointerNullabilityDiagnostic Diag) {
  switch (Diag.Code) {
  case PointerNullabilityDiagnostic::ErrorCode::ExpectedNonnull:
  case PointerNullabilityDiagnostic::ErrorCode::
      ExpectedNonnullWithCheckOnNonConstCall:
    return diagnosticExpectedNonNullMessage(Diag.Ctx);
  case PointerNullabilityDiagnostic::ErrorCode::InconsistentAnnotations:
    return "nullability annotations are inconsistent with a previous "
           "declaration";
  case PointerNullabilityDiagnostic::ErrorCode::
      InconsistentAnnotationsForParameter:
    return "nullability annotations for parameter are inconsistent with a "
           "previous declaration";
  case PointerNullabilityDiagnostic::ErrorCode::
      InconsistentAnnotationsForReturn:
    return "nullability annotations for return type are inconsistent with a "
           "previous declaration";
  case PointerNullabilityDiagnostic::ErrorCode::
      AccessingMovedFromNonnullPointer:
    return "reading from a Nonnull pointer that was initialized or set to "
           "null (possibly because it was moved from)";
  case PointerNullabilityDiagnostic::ErrorCode::
      NonnullPointerFieldNullableAtExit:
    return "a Nonnull pointer field is null when the method exits "
           "(possibly because it was moved from)";
  case PointerNullabilityDiagnostic::ErrorCode::Untracked:
    return "internal error: pointer expression not modeled";
  case PointerNullabilityDiagnostic::ErrorCode::AssertFailed:
    return "nullability does not match assertion";
  case PointerNullabilityDiagnostic::ErrorCode::ExpectedEqualNullability:
    return diagnosticExpectedEqualMessage(Diag.Ctx);
  }
}

static std::unique_ptr<dataflow::Solver> makeSatSolver() {
  return clang::dataflow::nullability::makeDefaultSolverForDiagnosis();
}

namespace {
constexpr StringLiteral kValueDeclID("vd");
} // namespace

void PointerNullabilityCheck::registerMatchers(MatchFinder *Finder) {
  using namespace ::clang::ast_matchers; // NOLINT: Too many names.

  auto Matcher =
      valueDecl(anyOf(functionDecl(unless(isDefaulted())),
                      varDecl(unless(hasDeclContext(functionDecl()))),
                      fieldDecl(hasDeclContext(cxxRecordDecl()))),
                unless(isExpansionInSystemHeader()), unless(isImplicit()))
          .bind(kValueDeclID);
  Finder->addMatcher(Matcher, this);
}

void PointerNullabilityCheck::registerPPCallbacks(
    const SourceManager &SM, Preprocessor *PP, Preprocessor *ModuleExpanderPP) {
  // We need to see pragmas within headers inside modules, if possible.
  clang::dataflow::nullability::registerPragmaHandler(*ModuleExpanderPP,
                                                      Pragmas);
}

void PointerNullabilityCheck::onEndOfTranslationUnit() {
  // Clear filter that caches the source manager and main file information.
  InMainFileOrHeaderFilter = nullptr;
}

void PointerNullabilityCheck::initializeLocFilter(
    const MatchFinder::MatchResult &Result, SourceManager &SM) {
  if (CheckMainFileOnly) {
    InMainFileOrHeaderFilter = clang::dataflow::nullability::getLocFilter(
        SM, clang::dataflow::nullability::LocFilterKind::kMainFileOrHeader);
  }
}

void PointerNullabilityCheck::check(const MatchFinder::MatchResult &Result) {
  SourceManager &SM = *Result.SourceManager;

  if (SM.getDiagnostics().hasUncompilableErrorOccurred())
    return;

  const auto *VD = Result.Nodes.getNodeAs<ValueDecl>(kValueDeclID);

  if (InMainFileOrHeaderFilter == nullptr)
    initializeLocFilter(Result, SM);
  if (InMainFileOrHeaderFilter != nullptr &&
      !InMainFileOrHeaderFilter->check(VD->getBeginLoc())) {
    return;
  }

  auto Diags = clang::dataflow::nullability::diagnosePointerNullability(
      VD, Pragmas, makeSatSolver);
  if (!Diags) {
    diag(VD->getBeginLoc(), "internal error: %0") << Diags.takeError();
    return;
  }

  for (const PointerNullabilityDiagnostic &Diag : *Diags) {
    if (shouldReportError(Diag.Code)) {
      diag(Diag.Range.getBegin(), diagnosticMessage(Diag))
          << Diag.Range << (Diag.ParamName != nullptr) << Diag.ParamName
          << (Diag.Callee != nullptr) << Diag.Callee;
      if (Diag.NoteRange.isValid() && !Diag.NoteMessage.empty()) {
        diag(Diag.NoteRange.getBegin(), Diag.NoteMessage, DiagnosticIDs::Note)
            << Diag.NoteRange;
      }
    }
  }
}

} // namespace clang::tidy::bugprone
