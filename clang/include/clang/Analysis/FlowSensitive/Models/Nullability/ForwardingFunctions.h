// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// This file provides utilities to detect forwarding functions like
// `std::make_unique` and find the underlying function/constructor call.
// The underlying function has more interesting nullability annotations or
// allows inference to be more precise than analyzing the forwarding function
// itself.

#ifndef THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_FORWARDINGFUNCTIONS_H
#define THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_FORWARDINGFUNCTIONS_H

#include "clang/AST/Decl.h"

namespace clang::dataflow::nullability {

// Returns the Initializer from the underlying `new` expression in
// `std::make_unique`. E.g., the initializer is often a `CXXConstructExpr` (but
// can be other expressions), given that `Decl` is an instantiation of
// `std::make_unique` with an interesting template type parameter. Primitives,
// and array template type parameter are not interesting because: (a) there is
// no constructor call to analyze or (b) it is just a 0-arg constructor call.
// The 0-arg constructor call may leave Nonnull fields uninitialized (with
// indeterminate values). We can try to diagnose later during initialization.
// Returns `nullptr` otherwise.
const Expr *getUnderlyingInitExprInStdMakeUnique(const FunctionDecl &Decl);

// Returns the last forwarding function layer in the call chain starting
// with `FD`, if `FD` is considered a forwarding function like
// `std::make_unique`.
const FunctionDecl *getLastForwardingFunctionLayer(const FunctionDecl &Decl);

} // namespace clang::dataflow::nullability

#endif // THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_FORWARDINGFUNCTIONS_H
