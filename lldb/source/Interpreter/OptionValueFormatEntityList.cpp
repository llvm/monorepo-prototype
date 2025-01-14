//===-- OptionValueFormatEntityList.cpp
//---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Interpreter/OptionValueFormatEntityList.h"

#include "lldb/Core/Module.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StringList.h"
using namespace lldb;
using namespace lldb_private;

OptionValueFormatEntityList::OptionValueFormatEntityList() {}

void OptionValueFormatEntityList::Clear() {
  m_current_entries.clear();
  m_current_formats.clear();
  m_value_was_set = false;
}

static void EscapeBackticks(llvm::StringRef str, std::string &dst) {
  dst.clear();
  dst.reserve(str.size());

  for (size_t i = 0, e = str.size(); i != e; ++i) {
    char c = str[i];
    if (c == '`') {
      if (i == 0 || str[i - 1] != '\\')
        dst += '\\';
    }
    dst += c;
  }
}

void OptionValueFormatEntityList::DumpValue(const ExecutionContext *exe_ctx,
                                            Stream &strm, uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    const bool one_line = dump_mask & eDumpOptionCommand;
    const size_t size = m_current_formats.size();
    if (dump_mask & eDumpOptionType)
      strm.Printf(" =%s", (size > 0 && !one_line) ? "\n" : "");
    if (!one_line)
      strm.IndentMore();
    for (uint32_t i = 0; i < size; ++i) {
      if (!one_line) {
        strm.Indent();
        strm.Printf("[%u]: ", i);
      }
      strm << m_current_formats[i];
      if (one_line)
        strm << ' ';
    }
    if (!one_line)
      strm.IndentLess();
  }
}

static llvm::Expected<FormatEntity::Entry> Parse(llvm::StringRef str) {
  FormatEntity::Entry entry;
  Status error = FormatEntity::Parse(str, entry);
  if (error.Fail())
    return error.ToError();
  return entry;
}

llvm::Error OptionValueFormatEntityList::Append(llvm::StringRef str) {
  auto maybe_entry = Parse(str);
  if (!maybe_entry)
    return maybe_entry.takeError();

  m_current_entries.emplace_back(*maybe_entry);
  m_current_formats.emplace_back(str);

  return llvm::Error::success();
}

llvm::Error OptionValueFormatEntityList::Insert(size_t idx,
                                                llvm::StringRef str) {
  if (idx >= m_current_formats.size())
    return llvm::createStringError(
        "invalid file list index %s, index must be 0 through %u", idx,
        m_current_formats.size());

  auto maybe_entry = Parse(str);
  if (!maybe_entry)
    return maybe_entry.takeError();

  m_current_entries.insert(m_current_entries.begin() + idx, *maybe_entry);
  m_current_formats.insert(m_current_formats.begin() + idx, std::string(str));

  return llvm::Error::success();
  return llvm::Error::success();
}

llvm::Error OptionValueFormatEntityList::Replace(size_t idx,
                                                 llvm::StringRef str) {
  if (idx >= m_current_formats.size())
    return llvm::createStringError(
        "invalid file list index %s, index must be 0 through %u", idx,
        m_current_formats.size());

  auto maybe_entry = Parse(str);
  if (!maybe_entry)
    return maybe_entry.takeError();

  m_current_entries[idx] = *maybe_entry;
  m_current_formats[idx] = str;

  return llvm::Error::success();
}

llvm::Error OptionValueFormatEntityList::Remove(size_t idx) {
  if (idx >= m_current_formats.size())
    return llvm::createStringError(
        "invalid fromat entry list index %s, index must be 0 through %u", idx,
        m_current_formats.size());

  m_current_formats.erase(m_current_formats.begin() + idx);
  m_current_entries.erase(m_current_entries.begin() + idx);

  return llvm::Error::success();
}

Status
OptionValueFormatEntityList::SetValueFromString(llvm::StringRef value_str,
                                                VarSetOperationType op) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  Args args(value_str.str());
  const size_t argc = args.GetArgumentCount();

  switch (op) {
  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:

  case eVarSetOperationAssign:
    Clear();
    // Fall through to append case
    [[fallthrough]];
  case eVarSetOperationAppend:
    if (argc > 0) {
      m_value_was_set = true;
      for (size_t i = 0; i < argc; ++i) {
        if (llvm::Error err = Append(args.GetArgumentAtIndex(i)))
          return Status::FromError(std::move(err));
      }
      NotifyValueChanged();
    } else {
      return Status::FromErrorString(
          "assign operation takes at least one file path argument");
    }
    break;
  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
    if (argc > 1) {
      uint32_t idx;
      if (!llvm::to_integer(args.GetArgumentAtIndex(0), idx))
        return Status::FromErrorStringWithFormat("invalid index %s",
                                                 args.GetArgumentAtIndex(0));
      if (op == eVarSetOperationInsertAfter)
        ++idx;
      for (size_t i = 1; i < argc; ++i, ++idx) {
        if (llvm::Error err = Append(args.GetArgumentAtIndex(i)))
          return Status::FromError(std::move(err));
      }
      NotifyValueChanged();
    } else {
      return Status::FromErrorString(
          "insert operation takes an array index followed by "
          "one or more values");
    }
    break;
  case eVarSetOperationRemove:
    if (argc > 0) {
      std::vector<int> indexes;
      for (size_t i = 0; i < argc; ++i) {
        int idx;
        if (!llvm::to_integer(args.GetArgumentAtIndex(i), idx))
          return Status::FromErrorStringWithFormat(
              "invalid array index '%s', aborting remove operation",
              args.GetArgumentAtIndex(i));
        indexes.push_back(idx);
      }

      if (indexes.empty())
        return Status::FromErrorString(
            "remove operation takes one or more array index");

      // Sort and then erase in reverse so indexes are always valid.
      llvm::sort(indexes);
      for (auto i : llvm::reverse(indexes)) {
        if (auto err = Remove(i))
          return Status::FromError(std::move(err));
      }
    } else {
      return Status::FromErrorString(
          "remove operation takes one or more array index");
    }
    break;
  case eVarSetOperationInvalid:
    return OptionValue::SetValueFromString(value_str, op);
    break;
  }

  return {};
}

OptionValueSP OptionValueFormatEntityList::Clone() const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  return Cloneable::Clone();
}
