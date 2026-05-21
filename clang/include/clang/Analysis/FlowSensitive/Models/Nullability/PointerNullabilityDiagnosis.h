// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_POINTERNULLABILITYDIAGNOSIS_H
#define LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_POINTERNULLABILITYDIAGNOSIS_H

#include <memory>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/PointerNullabilityAnalysis.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/Pragma.h"
#include "clang/Analysis/FlowSensitive/Solver.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Error.h"

namespace clang::dataflow::nullability {

/// Diagnoses a nullability-related issue in the associated CFG element.
struct PointerNullabilityDiagnostic {
  enum class ErrorCode {
    /// A nullable pointer was used where a nonnull pointer was expected.
    ExpectedNonnull,
    /// Similar to `ExpectedNonnull`, but the pointer comes from a non-const
    /// call and there is a null check on a similar call elsewhere.
    /// `NoteRange` refers to the null check.
    ExpectedNonnullWithCheckOnNonConstCall,
    /// Nullability annotations are inconsistent with a previous declaration.
    /// `NoteRange` refers to the location of the previous declaration.
    InconsistentAnnotations,
    InconsistentAnnotationsForParameter,
    InconsistentAnnotationsForReturn,
    /// A moved-from nonnull pointer was accessed.
    AccessingMovedFromNonnullPointer,
    /// A nonnull pointer field is nullable at method exit.
    /// `NoteRange` refers to the field declaration.
    NonnullPointerFieldNullableAtExit,
    /// A pointer-typed expression was encountered with no corresponding model.
    Untracked,
    /// A nullability assertion was violated.
    AssertFailed,
    /// Two types were expected to have the same nullability, but they did not.
    ExpectedEqualNullability,
  };
  ErrorCode Code;
  /// Context in which the error occurred.
  enum class Context {
    /// Dereferencing a pointer.
    NullableDereference,
    /// Initializing a variable.
    Initializer,
    /// Assigning to a pointer.
    Assignment,
    /// Value of a return statement.
    ReturnValue,
    /// Function argument.
    FunctionArgument,
    Other
  } Ctx = Context::Other;
  CharSourceRange Range;
  /// The function where the argument is being passed to.
  /// Populated only if `Ctx` is `FunctionArgument`.
  const clang::NamedDecl *Callee = nullptr;
  /// Name of the parameter that the argument is being passed to.
  /// Populated only if `Ctx` is `FunctionArgument` and the parameter name is
  /// known.
  const clang::IdentifierInfo *ParamName = nullptr;
  /// Source range and message of a note to be emitted alongside the diagnostic.
  CharSourceRange NoteRange;
  std::string NoteMessage;
};

/// Creates a solver with default parameters that is suitable for passing to
/// `diagnosePointerNullability()`.
std::unique_ptr<dataflow::Solver> makeDefaultSolverForDiagnosis();

/// Checks that nullable pointers are used safely, using nullability information
/// that is collected by `PointerNullabilityAnalysis`.
///
/// Examples of null safety violations include dereferencing nullable pointers
/// without null checks, and assignments between pointers of incompatible
/// nullability.
///
/// If `VD` is not a function, this merely checks that the annotations on `VD`
/// are consistent with the annotations on its canonical declaration.
///
/// Returns an empty vector when no issues are found in the code.
llvm::Expected<llvm::SmallVector<PointerNullabilityDiagnostic>>
diagnosePointerNullability(
    const ValueDecl *VD, const NullabilityPragmas &Pragmas,
    const SolverFactory &MakeSolver = makeDefaultSolverForDiagnosis);

} // namespace clang::dataflow::nullability

#endif // LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_POINTERNULLABILITYDIAGNOSIS_H
