.. title:: clang-tidy - bugprone-smartptr-reset-ambiguous-call

bugprone-smartptr-reset-ambiguous-call
======================================

Finds potentially erroneous calls to ``reset`` method on
smart pointers when the pointee type also has a ``reset`` method.
Having ``reset`` method in both classes makes it easy to accidentally
make the pointer null when intending to reset the underlying object.

.. code-block:: c++

  struct Resettable {
    void reset() { /* Own reset logic */ }
  };

  auto ptr = std::make_unique<Resettable>();

  ptr->reset();  // Calls underlying reset method
  ptr.reset();   // Makes the pointer null

Both calls are valid C++ code, but the second one might not be
what the developer intended, as it destroys the pointed-to object
rather than resetting its state.
It's easy to make such typo because the difference between ``.`` and ``->`` is really small.

The recommended approach is to make the intent explicit by using either member access or direct assignment:

.. code-block:: c++

  std::unique_ptr<Resettable> ptr = std::make_unique<Resettable>();

  (*ptr).reset();  // Clearly calls underlying reset method
  ptr = nullptr;   // Clearly makes the pointer null