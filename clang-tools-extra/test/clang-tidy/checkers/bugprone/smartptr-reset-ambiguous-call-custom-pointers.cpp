// RUN: %check_clang_tidy %s bugprone-smartptr-reset-ambiguous-call %t \
// RUN: -config='{CheckOptions: \
// RUN:  {bugprone-smartptr-reset-ambiguous-call.SmartPointers: "::std::unique_ptr;boost::shared_ptr"}}' \
// RUN: --

namespace std {

template <typename T>
struct unique_ptr {
  T& operator*() const;
  T* operator->() const;
  void reset(T* p = nullptr);
};

template <typename T>
struct shared_ptr {
  T& operator*() const;
  T* operator->() const;
  void reset();
  void reset(T*);
};

} // namespace std

namespace boost {

template <typename T>
struct shared_ptr {
  T& operator*() const;
  T* operator->() const;
  void reset();
};

} // namespace boost

struct Resettable {
  void reset();
  void doSomething();
};

void Positive() {
  std::unique_ptr<Resettable> u;
  u.reset();
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: bugprone 'reset()' call on smart pointer
  // CHECK-FIXES: u = nullptr;
  u->reset();
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: bugprone 'reset()' call on smart pointer
  // CHECK-FIXES: (*u).reset();

  boost::shared_ptr<Resettable> s;
  s.reset();
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: bugprone 'reset()' call on smart pointer
  // CHECK-FIXES: s = nullptr;
  s->reset();
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: bugprone 'reset()' call on smart pointer
  // CHECK-FIXES: (*s).reset();
}

void Negative() {
  std::shared_ptr<Resettable> s_ptr;
  s_ptr.reset();
  s_ptr->reset();
  s_ptr->doSomething();

  std::unique_ptr<Resettable> u_ptr;
  u_ptr.reset(nullptr);
  u_ptr->doSomething();
}