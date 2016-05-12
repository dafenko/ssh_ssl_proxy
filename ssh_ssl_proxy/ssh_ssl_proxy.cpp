/*
 ssh_ssl_proxy.cpp

 Created on: 6.3.2016
 Author: Daniel Ferenci dafe@dafe.net

 This file is part of ssh_ssl_proxy.

 ssh_ssl_proxy is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ssh_ssl_proxy is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with ssh_ssl_proxy.  If not, see <http://www.gnu.org/licenses/>.

 Daniel Ferenci dafe@dafe.net 6.3.2016

 This file modifies file:
 */
// daemon.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <syslog.h>
#include <signal.h>
#include <string>
#include <fstream>

#include "ssh_ssl_proxy.h"
#include "configuration.h"
#include "bridge.h"

class pidfile
{
public:
	pidfile(const std::string file_path) : m_pidfile(file_path) {}
	void write() {
		// create pid file
		std::ofstream out(m_pidfile.data(), std::ios::trunc | std::ios::out);
		if( out.good() ) {
			out << getpid() << std::endl;
		} else {
			syslog(LOG_ERR | LOG_USER,
				"ssh_ssl_proxy: Can not write to pid file: %s", m_pidfile.data());
		}
	}

private:
	std::string m_pidfile;
};
int main(int argc, char* argv[]) {
	try {
		ssh_ssl_proxy::configuration config(argc, argv);
		config.load();

		boost::asio::io_service ios;

		ssh_ssl_proxy::bridge::acceptor acceptor(ios, config.local_host(),
				config.local_port(), config.forward_host(),
				config.forward_port_ssh(), config.forward_port_ssl());
		acceptor.accept_connections();

		boost::asio::signal_set signals(ios, SIGINT, SIGTERM);
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

		ios.notify_fork(boost::asio::io_service::fork_prepare);
		if (pid_t pid = fork()) {
			if (pid > 0) {
				// We're in the parent process and need to exit.
				exit(0);
			} else {
				syslog(LOG_ERR | LOG_USER,
						"ssh_ssl_proxy: First fork failed: %m");
				return 1;
			}
		}
		// Make the process a new session leader. This detaches it from the
		// terminal.
		setsid();
		// change dir to root to avoid unmounted file system issues
		if (chdir("/") != 0) {
			syslog(LOG_ERR | LOG_USER,
					"ssh_ssl_proxy: chdir failed.");
			return 1;
		}

		// The file mode creation mask is also inherited from the parent process.
		// We don't want to restrict the permissions on files created by the
		// daemon, so the mask is cleared.
		umask(0);

		// A second fork ensures the process cannot acquire a controlling terminal.
		if (pid_t pid = fork()) {
			if (pid > 0) {
				exit(0);
			} else {
				syslog(LOG_ERR | LOG_USER,
						"ssh_ssl_proxy: Second fork failed: %m");
				return 1;
			}
		}

		pidfile pidf(config.pid_file());
		pidf.write();

		// Close the standard streams. This decouples the daemon from the terminal
		// that started it.
		close(0);
		close(1);
		close(2);

		// We don't want the daemon to have any standard input.
		if (open("/dev/null", O_RDONLY) < 0) {
			syslog(LOG_ERR | LOG_USER,
					"ssh_ssl_proxy: Unable to open /dev/null: %m");
			return 1;
		}

		// Send standard output to a log file.
		const char* output = "/var/log/ssh_ssl_proxy.log";
		const int flags = O_WRONLY | O_CREAT | O_APPEND;
		const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		if (open(output, flags, mode) < 0) {
			syslog(LOG_ERR | LOG_USER,
					"ssh_ssl_proxy: Unable to open output file %s: %m", output);
			return 1;
		}

		// Also send standard error to the same log file.
		if (dup(1) < 0) {
			syslog(LOG_ERR | LOG_USER,
					"ssh_ssl_proxy: Unable to dup output descriptor: %m");
			return 1;
		}



		// Inform the io_service that we have finished becoming a daemon. The
		// io_service uses this opportunity to create any internal file descriptors
		// that need to be private to the new process.
		ios.notify_fork(boost::asio::io_service::fork_child);

		// The io_service can now be used normally.
		syslog(LOG_INFO | LOG_USER, "ssh_ssl_proxy: Daemon started");
		ios.run();
		syslog(LOG_INFO | LOG_USER, "ssh_ssl_proxy: Daemon stopped");
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
