// RUN: %check_clang_tidy %s readability-stringview-substr %t

namespace std {
template <typename T>
class basic_string_view {
public:
  using size_type = unsigned long;
  static constexpr size_type npos = -1;

  basic_string_view(const char*) {}
  basic_string_view substr(size_type pos, size_type count = npos) const { return *this; }
  void remove_prefix(size_type n) {}
  void remove_suffix(size_type n) {}
  size_type length() const { return 0; }

  // Needed for assignment operator
  basic_string_view& operator=(const basic_string_view&) { return *this; }
};

using string_view = basic_string_view<char>;
}

void test() {
  std::string_view sv("test");
  std::string_view sv1("test");
  std::string_view sv2("other");

  // Should match: Removing N characters from start
  sv = sv.substr(5);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_prefix' over 'substr' for removing characters from the start [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_prefix(5)

  // Should match: Removing N characters from end
  sv = sv.substr(0, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  // Should not match: Basic substring operations
  sv = sv.substr(0, 3);  // No warning - taking first N characters
  sv = sv.substr(1, 3);  // No warning - taking N characters from position 1
  
  // Should not match: Operations on different string_views
  sv = sv2.substr(0, sv2.length() - 3);  // No warning - not self-assignment
  sv = sv.substr(0, sv2.length() - 3);   // No warning - length() from different string_view
  sv1 = sv1.substr(0, sv2.length() - 3); // No warning - length() from different string_view
  sv = sv1.substr(0, sv1.length() - 3);  // No warning - not self-assignment

  // Should not match: Not actually removing from end
  sv = sv.substr(0, sv.length());       // No warning - keeping entire string
  sv = sv.substr(0, sv.length() - 0);   // No warning - subtracting zero
  
  // Should not match: Complex expressions
  sv = sv.substr(0, sv.length() - (3 + 2));  // No warning - complex arithmetic
  sv = sv.substr(1 + 2, sv.length() - 3);    // No warning - complex start position
}

void test_zeros() {
  std::string_view sv("test");
  const int kZero = 0;
  constexpr std::string_view::size_type Zero = 0;  // Fixed: using string_view::size_type
  #define START_POS 0
  
  // All of these should match remove_suffix pattern and trigger warnings
  sv = sv.substr(0, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  sv = sv.substr(kZero, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  sv = sv.substr(Zero, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  sv = sv.substr(START_POS, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  sv = sv.substr((0), sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  sv = sv.substr(0u, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  sv = sv.substr(0UL, sv.length() - 3);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: prefer 'remove_suffix' over 'substr' for removing characters from the end [readability-stringview-substr]
  // CHECK-FIXES: sv.remove_suffix(3)

  // These should NOT match the remove_suffix pattern
  sv = sv.substr(1-1, sv.length() - 3);  // No warning - complex expression
  sv = sv.substr(sv.length() - 3, sv.length() - 3);  // No warning - non-zero start
}

