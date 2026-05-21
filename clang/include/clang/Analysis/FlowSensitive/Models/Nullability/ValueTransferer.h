// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_VALUETRANSFERER_H
#define THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_VALUETRANSFERER_H

#include "clang/AST/Expr.h"
#include "clang/Analysis/CFG.h"
#include "clang/Analysis/FlowSensitive/CFGMatchSwitch.h"
#include "clang/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "clang/Analysis/FlowSensitive/MatchSwitch.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/PointerNullabilityLattice.h"
#include "clang/Analysis/FlowSensitive/Value.h"

namespace clang::dataflow::nullability {
// Returns a switch over CFG elements that can be used to transfer
// (flow-sensitive) value properties, i.e. to update the analysis state to
// account for the impact of the CFG element.
dataflow::CFGMatchSwitch<dataflow::TransferState<PointerNullabilityLattice>>
buildValueTransferer();

// If `E` is already associated with a `PointerValue`, returns it.
// Otherwise, associates a newly created `PointerValue` with `E` and returns it.
// Returns null iff `E` is not a raw pointer expression.
dataflow::PointerValue *ensureRawPointerHasValue(const Expr *E,
                                                 dataflow::Environment &Env);

// Initialize the null state of `PointerVal` based on the nullability of the
// type of `E`.
void initPointerFromTypeNullability(
    dataflow::PointerValue &PointerVal, const Expr *E,
    dataflow::TransferState<PointerNullabilityLattice> &State);

// Ensure that all expressions of smart pointer type have an underlying
// raw pointer initialized from the type nullability.
void ensureSmartPointerInitialized(
    const CFGElement &Elt,
    dataflow::TransferState<PointerNullabilityLattice> &State);
} // namespace clang::dataflow::nullability

#endif // THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_VALUETRANSFERER_H
