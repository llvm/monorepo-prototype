================
TypeSanitizer
================

.. contents::
   :local:

Introduction
============

TypeSanitizer is a detector for strict type aliasing violations. It consists of a compiler
instrumentation module and a run-time library. The tool detects violations where you access 
memory under a different type than the dynamic type of the object.

The violations TypeSanitizer catches may cause the compiler to emit incorrect code.

As TypeSanitizer is still experimental, it can currently have a large impact on runtime speed, 
memory use, and code size.

How to build
============

Build LLVM/Clang with `CMake <https://llvm.org/docs/CMake.html>`_ and enable
the ``compiler-rt`` runtime. An example CMake configuration that will allow
for the use/testing of TypeSanitizer:

.. code-block:: console

   $ cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="compiler-rt" <path to source>/llvm

Usage
=====

Compile and link your program with ``-fsanitize=type`` flag.  The
TypeSanitizer run-time library should be linked to the final executable, so
make sure to use ``clang`` (not ``ld``) for the final link step. To
get a reasonable performance add ``-O1`` or higher.
TypeSanitizer by default doesn't print the full stack trace on error messages. Use ``TYSAN_OPTIONS=print_stacktrace=1`` 
to print the full trace. To get nicer stack traces in error messages add ``-fno-omit-frame-pointer`` and 
``-g``.  To get perfect stack traces you may need to disable inlining (just use ``-O1``) and tail call elimination 
(``-fno-optimize-sibling-calls``).

.. code-block:: console

    % cat example_AliasViolation.c
    int main(int argc, char **argv) {
      int x = 100;
      float *y = (float*)&x;
      *y += 2.0f;          // Strict aliasing violation
      return 0;
    }

    # Compile and link
    % clang++ -g -fsanitize=type example_AliasViolation.cc

The program will print an error message to stderr each time a strict aliasing violation is detected. 
The program won't terminate, which will allow you to detect many strict aliasing violations in one 
run.

.. code-block:: console

    % ./a.out
    ==1375532==ERROR: TypeSanitizer: type-aliasing-violation on address 0x7ffeebf1a72c (pc 0x5b3b1145ff41 bp 0x7ffeebf1a660 sp 0x7ffeebf19e08 tid 1375532)
    READ of size 4 at 0x7ffeebf1a72c with type float accesses an existing object of type int
        #0 0x5b3b1145ff40 in main example_AliasViolation.c:4:10

    ==1375532==ERROR: TypeSanitizer: type-aliasing-violation on address 0x7ffeebf1a72c (pc 0x5b3b1146008a bp 0x7ffeebf1a660 sp 0x7ffeebf19e08 tid 1375532)
    WRITE of size 4 at 0x7ffeebf1a72c with type float accesses an existing object of type int
        #0 0x5b3b11460089 in main example_AliasViolation.c:4:10

Error terminology
------------------

There are some terms that may appear in TypeSanitizer errors that are derived from 
`TBAA Metadata <https://llvm.org/docs/LangRef.html#tbaa-metadata>`. This section hopes to provide a 
brief dictionary of these terms.

* ``omnipotent char``: This is a special type which can alias with anything. Its name comes from the C/C++ 
  type ``char``.
* ``type p[x]``: This signifies pointers to the type. x is the number of indirections to reach the final value.
  As an example, a pointer to a pointer to an integer would be ``type p2 int``.

TypeSanitizer is still experimental. User-facing error messages should be improved in the future to remove 
references to LLVM IR specific terms.

Sanitizer features
==================

``__has_feature(type_sanitizer)``
------------------------------------

In some cases one may need to execute different code depending on whether
TypeSanitizer is enabled.
:ref:`\_\_has\_feature <langext-__has_feature-__has_extension>` can be used for
this purpose.

.. code-block:: c

    #if defined(__has_feature)
    #  if __has_feature(type_sanitizer)
    // code that builds only under TypeSanitizer
    #  endif
    #endif

``__attribute__((no_sanitize("type")))``
-----------------------------------------------

Some code you may not want to be instrumented by TypeSanitizer.  One may use the
function attribute ``no_sanitize("type")`` to disable instrumenting type aliasing. 
It is possible, depending on what happens in non-instrumented code, that instrumented code 
emits false-positives/ false-negatives. This attribute may not be supported by other 
compilers, so we suggest to use it together with ``__has_feature(type_sanitizer)``.

``__attribute__((disable_sanitizer_instrumentation))``
--------------------------------------------------------

The ``disable_sanitizer_instrumentation`` attribute can be applied to functions
to prevent all kinds of instrumentation. As a result, it may introduce false
positives and incorrect stack traces. Therefore, it should be used with care,
and only if absolutely required; for example for certain code that cannot
tolerate any instrumentation and resulting side-effects. This attribute
overrides ``no_sanitize("type")``.

Ignorelist
----------

TypeSanitizer supports ``src`` and ``fun`` entity types in
:doc:`SanitizerSpecialCaseList`, that can be used to suppress aliasing 
violation reports in the specified source files or functions. Like 
with other methods of ignoring instrumentation, this can result in false 
positives/ false-negatives.

Limitations
-----------

* TypeSanitizer uses more real memory than a native run. It uses 8 bytes of
  shadow memory for each byte of user memory.
* There are transformation passes which run before TypeSanitizer. If these 
  passes optimize out an aliasing violation, TypeSanitizer cannot catch it.
* Currently, all instrumentation is inlined. This can result in a **15x** 
  (on average) increase in generated file size, and **3x** to **7x** increase 
  in compile time. In some documented cases this can cause the compiler to hang.
  There are plans to improve this in the future.
* Codebases that use unions and struct-initialized variables can see incorrect 
  results, as TypeSanitizer doesn't yet instrument these reliably.

Current Status
--------------

TypeSanitizer is brand new, and still in development. There are some known 
issues, especially in areas where Clang's emitted TBAA data isn't extensive 
enough for TypeSanitizer's runtime.

We are actively working on enhancing the tool --- stay tuned.  Any help, 
issues, pull requests, ideas, is more than welcome. You can find the 
`issue tracker here.<https://github.com/llvm/llvm-project/issues?q=is%3Aissue%20state%3Aopen%20TySan%20label%3Acompiler-rt%3Atysan>`
