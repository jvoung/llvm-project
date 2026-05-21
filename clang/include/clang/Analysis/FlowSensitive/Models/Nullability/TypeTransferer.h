// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_TYPETRANSFERER_H
#define THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_TYPETRANSFERER_H

#include "clang/Analysis/FlowSensitive/CFGMatchSwitch.h"
#include "clang/Analysis/FlowSensitive/MatchSwitch.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/PointerNullabilityLattice.h"

namespace clang::dataflow::nullability {
// Returns a switch over CFG elements that can be used to transfer
// (non-flow-sensitive) type properties, i.e. to update the analysis state to
// account for the impact of the CFG element.
dataflow::CFGMatchSwitch<dataflow::TransferState<PointerNullabilityLattice>>
buildTypeTransferer();

} // namespace clang::dataflow::nullability

#endif // THIRD_PARTY_LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_TYPETRANSFERER_H
