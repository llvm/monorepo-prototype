//===-- OutputRedirector.cpp -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "DAP.h"
#include "OutputRedirector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <thread>
#include <utility>
#include <vector>

using namespace llvm;

namespace lldb_dap {

OutputRedirector::~OutputRedirector() {
  // Make a best effort cleanup of any redirected FDs.
  for (auto &[from, to] : m_redirects)
    ::dup2(to, from); // ignoring errors

  for (const auto &fd : m_fds)
    close(fd);
}

Expected<int>
OutputRedirector::RedirectFd(int fd,
                             std::function<void(llvm::StringRef)> callback) {
  int new_fd[2];
#if defined(_WIN32)
  if (_pipe(new_fd, 4096, O_TEXT) == -1)
#else
  if (pipe(new_fd) == -1)
#endif
    return createStringError(llvm::errnoAsErrorCode(),
                             "Couldn't create new pipe for fd %d.", fd);

  m_fds.push_back(new_fd[0]);
  m_fds.push_back(new_fd[1]);

  if (fd != -1) {
    if (dup2(new_fd[1], fd) == -1)
      return createStringError(llvm::errnoAsErrorCode(),
                               "Couldn't override the fd %d.", fd);

    m_redirects.push_back(std::make_pair(new_fd[1], fd));
  }

  int read_fd = new_fd[0];
  std::thread t([read_fd, callback]() {
    char buffer[OutputBufferSize];
    while (true) {
      ssize_t bytes_count = read(read_fd, &buffer, sizeof(buffer));
      if (bytes_count == 0)
        return;
      if (bytes_count == -1) {
        if (errno == EAGAIN || errno == EINTR)
          continue;
        break;
      }
      callback(StringRef(buffer, bytes_count));
    }
  });
  t.detach();
  return new_fd[1];
}

} // namespace lldb_dap
