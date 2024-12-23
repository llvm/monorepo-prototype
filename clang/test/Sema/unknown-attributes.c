// RUN: %clang_cc1 -fsyntax-only -std=c23 -verify=pedantic -pedantic %s
// RUN: %clang_cc1 -fsyntax-only -std=c23 -verify %s

// expected-no-diagnostics

[[unknown::a(b((c)) d(e((f)))), unknown::g(h k)]] // pedantic-warning {{unknown attribute 'a' ignored}} \
                                                  // pedantic-warning {{unknown attribute 'g' ignored}}
int main(void) {
    return 0;
}
