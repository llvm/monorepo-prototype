//===-- OptionValueFormatEntityList.h --------------------------------*-
// C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_INTERPRETER_OPTIONVALUEFORMATENTITYLIST_H
#define LLDB_INTERPRETER_OPTIONVALUEFORMATENTITYLIST_H

#include "lldb/Core/FormatEntity.h"
#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueFormatEntityList
    : public Cloneable<OptionValueFormatEntityList, OptionValue> {
public:
  OptionValueFormatEntityList();

  OptionValueFormatEntityList(const OptionValueFormatEntityList &other)
      : Cloneable(other), m_current_entries(other.m_current_entries),
        m_current_formats(other.m_current_formats) {}

  ~OptionValueFormatEntityList() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeFormatEntityList; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override;

  std::vector<FormatEntity::Entry> GetCurrentValue() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_current_entries;
  }

protected:
  llvm::Error Append(llvm::StringRef str);
  llvm::Error Insert(size_t idx, llvm::StringRef str);
  llvm::Error Replace(size_t idx, llvm::StringRef str);
  llvm::Error Remove(size_t idx);

  lldb::OptionValueSP Clone() const override;

  std::vector<FormatEntity::Entry> m_current_entries;
  std::vector<std::string> m_current_formats;
  mutable std::recursive_mutex m_mutex;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEFORMATENTITYLIST_H
