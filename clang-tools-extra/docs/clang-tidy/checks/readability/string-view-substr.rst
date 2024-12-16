.. title:: clang-tidy - readability-string-view-substr

readability-string-view-substr
==============================

Finds ``string_view substr()`` calls that can be replaced with clearer alternatives
using ``remove_prefix()`` or ``remove_suffix()``.

The check suggests the following transformations:

===========================================  =======================
Expression                                   Replacement
===========================================  =======================
``sv = sv.substr(n)``                        ``sv.remove_prefix(n)``
``sv = sv.substr(0, sv.length()-n)``         ``sv.remove_suffix(n)``
===========================================  =======================
