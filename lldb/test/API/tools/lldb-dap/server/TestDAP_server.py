"""
Test lldb-dap server integration.
"""

import os
import tempfile

import dap_server
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
import lldbdap_testcase


class TestDAP_server(lldbdap_testcase.DAPTestCaseBase):
    def do_test_server(self, connection):
        log_file_path = self.getBuildArtifact("dap.txt")
        server, connection = dap_server.DebugAdaptorServer.launch(
            self.lldbDAPExec, connection, log_file=log_file_path
        )

        def cleanup():
            server.terminate()
            server.wait()

        self.addTearDownHook(cleanup)

        self.build()
        program = self.getBuildArtifact("a.out")
        source = "main.c"
        breakpoint_line = line_number(source, "// breakpoint")

        # Initial connection over the port.
        self.create_debug_adaptor(launch=False, connection=connection)
        self.launch(
            program,
            disconnectAutomatically=False,
        )
        self.set_source_breakpoints(source, [breakpoint_line])
        self.continue_to_next_stop()
        self.continue_to_exit()
        output = self.get_stdout()
        self.assertEquals(output, "hello world!\r\n")
        self.dap_server.request_disconnect()

        # Second connection over the port.
        self.create_debug_adaptor(launch=False, connection=connection)
        self.launch(program)
        self.set_source_breakpoints(source, [breakpoint_line])
        self.continue_to_next_stop()
        self.continue_to_exit()
        output = self.get_stdout()
        self.assertEquals(output, "hello world!\r\n")

    def test_server_port(self):
        """
        Test launching a binary with a lldb-dap in server mode on a specific port.
        """
        self.do_test_server(connection="tcp://localhost:0")

    @skipIfWindows
    def test_server_unix_socket(self):
        """
        Test launching a binary with a lldb-dap in server mode on a unix socket.
        """
        dir = tempfile.gettempdir()
        name = dir + "/dap-connection-" + str(os.getpid())

        def cleanup():
            os.unlink(name)

        self.addTearDownHook(cleanup)
        self.do_test_server(connection="unix://" + name)
