// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "clang/Analysis/FlowSensitive/Models/Nullability/PointerNullabilityLattice.h"

#include <cassert>
#include <functional>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Expr.h"
#include "clang/Analysis/FlowSensitive/ASTOps.h"
#include "clang/Analysis/FlowSensitive/DataflowLattice.h"
#include "clang/Analysis/FlowSensitive/Models/Nullability/TypeNullability.h"
#include "clang/Basic/LLVM.h"
#include "llvm/Support/ErrorHandling.h"

namespace clang::dataflow::nullability {
namespace {

using dataflow::LatticeJoinEffect;

// Returns overridden nullability information associated with a declaration.
// For now we only track top-level decl nullability symbolically.
const PointerTypeNullability *getDeclNullability(
    const Decl *D,
    const PointerNullabilityLattice::NonFlowSensitiveState &NFS) {
  if (!D)
    return nullptr;
  if (const auto *VD = dyn_cast_or_null<ValueDecl>(D->getCanonicalDecl())) {
    auto It = NFS.DeclTopLevelNullability.find(VD);
    if (It != NFS.DeclTopLevelNullability.end())
      return &It->second;
  }
  return nullptr;
}

} // namespace

const TypeNullability &
PointerNullabilityLatticeBase::insertExprNullabilityIfAbsent(
    const Expr *E, const std::function<TypeNullability()> &GetNullability) {
  E = &dataflow::ignoreCFGOmittedNodes(*E);
  if (auto It = NFS.ExprToNullability.find(E);
      It != NFS.ExprToNullability.end())
    return It->second;
  // Deliberately perform a separate lookup after calling GetNullability.
  // It may invalidate iterators, e.g. inserting missing vectors for children.
  auto [Iterator, Inserted] =
      NFS.ExprToNullability.insert({E, GetNullability()});
  if (!Inserted)
    reportFatalInternalError("GetNullability inserted same " +
                             Twine(E->getStmtClassName()));
  return Iterator->second;
}

void PointerNullabilityLatticeBase::overrideNullabilityFromDecl(
    const Decl *D, TypeNullability &N) const {
  // For now, overrides are always for pointer values only, and override only
  // the top-level nullability.
  if (N.empty())
    return;
  if (auto *PN = getDeclNullability(D, NFS)) {
    N.front() = *PN;
  }
}

LatticeJoinEffect PointerNullabilityLatticeBase::join(
    const PointerNullabilityLatticeBase &Other) {
  return LatticeJoinEffect::Unchanged;
}

} // namespace clang::dataflow::nullability
