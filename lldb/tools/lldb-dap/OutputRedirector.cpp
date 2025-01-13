//===-- OutputRedirector.cpp -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/

#include "OutputRedirector.h"
#include "DAP.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <system_error>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

using lldb_private::Pipe;
using lldb_private::Status;
using llvm::createStringError;
using llvm::Error;
using llvm::Expected;
using llvm::StringRef;

namespace lldb_dap {

Expected<int> OutputRedirector::GetWriteFileDescriptor() {
  if (m_fd == -1)
    return createStringError(std::errc::bad_file_descriptor,
                             "write handle is not open for writing");

  return m_fd;
}

Error OutputRedirector::RedirectTo(std::function<void(StringRef)> callback) {
  int new_fd[2] = {0};
#if defined(_WIN32)
  if (::_pipe(new_fd, 4096, O_TEXT) == -1) {
#else
  if (::pipe(new_fd) == -1) {
#endif
    int error = errno;
    return createStringError(llvm::inconvertibleErrorCode(),
                             "Couldn't create new pipe: %s", strerror(error));
  }

  int read_fd = new_fd[0];
  m_fd = new_fd[1];

  m_forwarder = std::thread([this, read_fd, callback]() {
    char buffer[OutputBufferSize];
    while (!m_stopped) {
      ssize_t bytes_read = ::read(read_fd, &buffer, sizeof(buffer));
      // EOF detected
      if (bytes_read == 0 || m_stopped)
        break;
      if (bytes_read == -1) {
        if (errno == EAGAIN || errno == EINTR)
          continue;
        break;
      }

      callback(StringRef(buffer, bytes_read));
    }
  });

  return Error::success();
}

void OutputRedirector::Stop() {
  m_stopped = true;

  if (m_fd != -1) {
    // Closing the pipe may not be sufficient to wake up the thread in case the
    // write descriptor is duplicated (to stdout/err or to another process).
    // Write a null byte to ensure the read call returns.
    char buf[] = "\0";
    ::write(m_fd, buf, sizeof(buf));
    ::close(m_fd);
    m_fd = -1;
    m_forwarder.join();
  }
}

} // namespace lldb_dap
