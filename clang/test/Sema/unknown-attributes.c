// RUN: %clang_cc1 -fsyntax-only -std=c23 -verify %s

[[unknown::a(b((c)) d(e((f)))), unknown::g(h k)]]
int a(void) {
    return 0;
}

[[clang::a(b((c)) d(e((f)))), clang::g(h k)]] // expected-warning {{unknown attribute 'a' ignored}} \
                                              // expected-warning {{unknown attribute 'g' ignored}}
int b(void) {
    return 0;
}
