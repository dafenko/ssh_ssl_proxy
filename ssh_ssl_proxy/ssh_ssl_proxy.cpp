/**
 *
 */

#include <syslog.h>
#include <signal.h>

#include "ssh_ssl_proxy.h"
#include "configuration.h"
#include "bridge.h"

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
		// change dir to root to avaoi mounting issues
		chdir("/");

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
