// RUN: %check_clang_tidy -std=c++17 %s bugprone-pointer-nullability %t --

// This file includes a minimal test suite to ensure that the check integrated
// with the analysis correctly. However, we leave the vast majority of tests to
// the unit tests of the analysis, rather than duplicating them here.

#define absl_nullable _Nullable

int positive_deref(int* _Nullable p) {
  return *p;
// CHECK-MESSAGES: :[[@LINE-1]]:11: warning: dereferencing a potentially nullable pointer [bugprone-pointer-nullability]
}

int positive_deref_macro(int *absl_nullable p) {
  return *p;
// CHECK-MESSAGES: :[[@LINE-1]]:11: warning: dereferencing a potentially nullable pointer
}

void use_nonnull(int* _Nonnull p, int* _Nullable n, int *_Nonnull q, int *_Nonnull);

void positive_bind_nonnull(int* _Nullable p) {
  use_nonnull(p, nullptr, nullptr, p);
// CHECK-MESSAGES: :[[@LINE-1]]:15: warning: expected parameter 'p' of 'use_nonnull' to be nonnull,
// CHECK-MESSAGES: :[[@LINE-2]]:27: warning: expected parameter 'q' of 'use_nonnull' to be nonnull,
// CHECK-MESSAGES: :[[@LINE-3]]:36: warning: expected unnamed parameter of 'use_nonnull' to be nonnull,
}

int *_Nonnull positive_return_null(int *_Nullable p) {
  return p;
// CHECK-MESSAGES: :[[@LINE-1]]:10: warning: returning a nullable pointer in a
}

struct Outer {
  struct Inner {
    int value;
  };
  Inner* _Nullable inner;
};

void positive_arrow_deref(Outer* _Nonnull outer) {
  outer->inner->value = 10;
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: dereferencing a potentially nullable pointer
}

void negative_checked_arrow_deref(Outer* _Nonnull outer) {
  if (outer->inner != nullptr)
    outer->inner->value = 10;
}

typedef void (*takeNonnullF)(int *_Nonnull p);
void target(takeNonnullF takeNonnull) {
  takeNonnull(nullptr);
// CHECK-MESSAGES: :[[@LINE-1]]:15: warning: expected unnamed parameter to be nonnull,
}

using Callback = void (*)();

void call_callback(_Nullable Callback callback) {
  callback();
// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: using a nullable pointer where a
}

struct NonConstCallNearMissTests {
  int *_Nullable get_nonconst();
  int *_Nullable get_nonconst2();
  NonConstCallNearMissTests* _Nullable get_self();
  int *_Nullable operator[](int i);
  int *_Nullable get_const() const;
  int *_Nonnull get_default();

  // check via != nullptr
  int ne_check() {
    if (get_nonconst() != nullptr)
      return *get_nonconst();
      // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: dereferencing a potentially
      // CHECK-MESSAGES: :[[@LINE-3]]:9: note: the nullable pointer comes from a
      // CHECK-MESSAGES-SAME: non-const method call, and there is a null check
      // CHECK-MESSAGES-SAME: on a similar call here. To show stability, capture
    return -1;
  }

  // check via == nullptr and `else`
  int eq_check() {
    if (nullptr == get_nonconst())
      return -1;
    return *get_nonconst();
    // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: dereferencing a potentially
    // CHECK-MESSAGES: :[[@LINE-4]]:9: note: the nullable pointer comes from a
  }

  // check via pointer to bool conversion
  int *_Nonnull bool_check() {
    // check via bool conversion
    if (get_nonconst())
      return get_nonconst();
      // CHECK-MESSAGES: :[[@LINE-1]]:14: warning: returning a nullable pointer
      // CHECK-MESSAGES: :[[@LINE-3]]:9: note: the nullable pointer comes from a
    return get_default();
  }

  // part of a more complex chain
  int *_Nonnull check_chained_with_deref() {
    // check via bool conversion
    if (get_self() != nullptr &&
        get_self()->get_const() != nullptr)
      // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: dereferencing a potentially
      // CHECK-MESSAGES: :[[@LINE-3]]:9: note: the nullable pointer comes from a
      return get_self()->get_const();
      // CHECK-MESSAGES: :[[@LINE-1]]:14: warning: dereferencing a potentially
      // CHECK-MESSAGES: :[[@LINE-6]]:9: note: the nullable pointer comes from a
      // CHECK-MESSAGES: :[[@LINE-3]]:14: warning: returning a nullable
    return get_default();
  }

  int operator_call_bool_check() {
    // check via bool conversion
    if ((*this)[1])
      return *(*this)[1];
      // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: dereferencing a potentially
      // CHECK-MESSAGES: :[[@LINE-3]]:9: note: the nullable pointer comes from a
    return -1;
  }

  int not_near_miss_not_flagged() {
    if (get_nonconst() != nullptr)
      return 0;
      // CHECK-MESSAGES-NOT: note: the nullable
    return -1;
  }
  int not_near_miss_not_flagged_const() {
    if (get_const() != nullptr)
      return *get_const();
    return -1;
  }
  int not_near_miss_diff_functions() {
    if (get_nonconst() != nullptr)
      return *get_nonconst2();
      // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: dereferencing a potentially
      // CHECK-MESSAGES-NOT: note: the nullable
    return -1;
  }
};

struct NonnullInitializationRawNoCtor {
  // Nonnull raw pointer fields explicitly initialized to null.
  int *_Nonnull nonnull1 = nullptr;
  // CHECK-MESSAGES: :[[@LINE-1]]:28: warning: initializing a Nonnull annotated variable with a nullable value
  // CHECK-MESSAGES: :[[@LINE-2]]:3: note: This nonnull-annotated class member should have a nonnull initializer
  int *_Nullable nullable = nullptr;
  int *_Nonnull nonnull2 = nullptr;
  // CHECK-MESSAGES: :[[@LINE-1]]:28: warning: initializing a Nonnull annotated variable with a nullable value
  // CHECK-MESSAGES: :[[@LINE-2]]:3: note: This nonnull-annotated class member should have a nonnull initializer
};

void UseNonnullInitializationRawNoCtor(int x) {
  NonnullInitializationRawNoCtor empty_brace{};
  // CHECK-MESSAGES: :[[@LINE-1]]:46: warning: initializing a Nonnull annotated
  NonnullInitializationRawNoCtor brace_with_null{nullptr};
  // CHECK-MESSAGES: :[[@LINE-1]]:50: warning: initializing a Nonnull annotated
  // CHECK-MESSAGES: :[[@LINE-2]]:57: warning: initializing a Nonnull annotated
  NonnullInitializationRawNoCtor brace_with_first_okay{&x, nullptr, nullptr};
  // CHECK-MESSAGES: :[[@LINE-1]]:69: warning: initializing a Nonnull annotated
  NonnullInitializationRawNoCtor brace_with_second_okay{nullptr, nullptr, &x};
  // CHECK-MESSAGES: :[[@LINE-1]]:57: warning: initializing a Nonnull annotated
  NonnullInitializationRawNoCtor brace_with_both_okay{&x, nullptr, &x};
}

void DereferenceNonnullDefinedInLambda() {
  auto lambda = []() {
    int x = 1;
    int* _Nonnull p = &x;
    *p;
    // CHECK-MESSAGES-NOT: warning: dereferencing a potentially nullable pointer
  };
}

void DereferenceNullableDefinedInLambda() {
  auto lambda = []() {
    int* _Nullable p = nullptr;
    *p;
    // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: dereferencing a potentially nullable pointer
    // CHECK-MESSAGES-NOT: note: This pointer is captured and dereferenced in a lambda
  };
}

void DereferenceNonnullCapturedInLambdaByReference() {
  int x = 1;
  int* _Nonnull p = &x;
  auto lambda = [&p]() {
    *p;
    // CHECK-MESSAGES-NOT: warning: dereferencing a potentially nullable pointer
  };
}

void DereferenceNullableCapturedInLambdaByReference() {
  int* _Nullable p = nullptr;
  auto lambda = [&p]() {
    *p;
    // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: dereferencing a potentially nullable pointer
    // CHECK-MESSAGES: :[[@LINE-2]]:6: note: This pointer is captured and dereferenced in a lambda
  };
}
