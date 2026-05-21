// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_TYPEANDMAYBELOCVISITOR_H
#define LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_TYPEANDMAYBELOCVISITOR_H

#include <optional>

#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "llvm/Support/ErrorHandling.h"

namespace clang::dataflow::nullability {

// Dispatching requires that matching Type/TypeLoc subclasses are visited
// together, e.g. visiting a PointerType with an optional PointerTypeLoc.
#define DISPATCH(TYPE_CLASS)                                                   \
  return static_cast<ImplClass *>(this)->visit##TYPE_CLASS(                    \
      static_cast<const TYPE_CLASS *>(T),                                      \
      L ? std::optional<TYPE_CLASS##Loc>(                                      \
              L->getUnqualifiedLoc().castAs<TYPE_CLASS##Loc>())                \
        : std::nullopt)

/// An analog of TypeVisitor and TypeLocVisitor that simultaneously visits a
/// Type and an optional TypeLoc that corresponds to a spelling of that Type.
///
/// Uses the same dispatching and inheritance patterns as TypeVisitor and
/// TypeLocVisitor; see TypeVisitor.h for full documentation of the approach.
template <typename ImplClass, typename RetTy = void>
class TypeAndMaybeLocVisitor {
public:
  RetTy visit(const Type *T, std::optional<TypeLoc> L) {
    switch (T->getTypeClass()) {
#define ABSTRACT_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT)                                                    \
  case Type::CLASS:                                                            \
    DISPATCH(CLASS##Type);
#include "clang/AST/TypeNodes.inc"
    }
    llvm_unreachable("Unknown type class!");
  }

#define TYPE(CLASS, PARENT)                                                    \
  RetTy visit##CLASS##Type(const CLASS##Type *T,                               \
                           std::optional<CLASS##TypeLoc> L) {                  \
    DISPATCH(PARENT);                                                          \
  }
#include "clang/AST/TypeNodes.inc"

  RetTy visitType(const Type *, std::optional<TypeLoc> L) { return RetTy(); }
};

#undef DISPATCH
} // namespace clang::dataflow::nullability

#endif // LLVM_CLANG_ANALYSIS_FLOWSENSITIVE_MODELS_NULLABILITY_TYPEANDMAYBELOCVISITOR_H
