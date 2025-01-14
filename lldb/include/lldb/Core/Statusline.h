//===-- Statusline.h -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "lldb/Core/Debugger.h"
#include "llvm/ADT/SmallVector.h"
#include <string>
#ifndef LLDB_CORE_STATUSBAR_H
#define LLDB_CORE_STATUSBAR_H

namespace lldb_private {
class Statusline {
public:
  Statusline(Debugger &debugger);
  ~Statusline();

  void Enable();
  void Disable();

  void Clear();
  void Update();

  void ReportProgress(const ProgressEventData &data);

  void TerminalSizeChanged() { m_terminal_size_has_changed = 1; }

private:
  // Draw the statusline with the given text.
  void Draw(llvm::StringRef msg);

  // Update terminal dimensions.
  void UpdateTerminalProperties();

  // Set the scroll window to the given height.
  void SetScrollWindow(uint64_t height);

  // Write at the given column.
  void AddAtPosition(uint64_t col, llvm::StringRef str);

  // Clear the statusline (without redrawing the background).
  void Reset();

  bool IsSupported() const;

  lldb::thread_result_t StatuslineThread();

  Debugger &m_debugger;

  struct ProgressReport {
    uint64_t id;
    std::string message;
  };
  llvm::SmallVector<ProgressReport, 8> m_progress_reports;
  std::optional<ProgressReport> m_progress_report;

  volatile std::sig_atomic_t m_terminal_size_has_changed = 1;
  uint64_t m_terminal_width = 0;
  uint64_t m_terminal_height = 0;
  uint64_t m_scroll_height = 0;

  std::string m_ansi_prefix = "${ansi.bg.cyan}${ansi.fg.black}";
  std::string m_ansi_suffix = "${ansi.normal}";
  llvm::SmallVector<FormatEntity::Entry> m_components;
};
} // namespace lldb_private
#endif // LLDB_CORE_STATUSBAR_H
