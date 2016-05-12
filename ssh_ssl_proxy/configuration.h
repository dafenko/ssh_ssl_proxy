/*
  configuration.h

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
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "ssh_ssl_proxy.h"

namespace ssh_ssl_proxy {

class configuration {
public:
	explicit configuration();
	configuration(int argc, char* argv[]);
	virtual ~configuration();
	void load();
	void show_usage();
	unsigned short local_port(){return m_local_port;};
	std::string &local_host(){return m_local_host;};
	std::string &forward_host(){return m_forward_host;};
	unsigned short forward_port_ssh(){return m_forward_port_ssh;};
	unsigned short forward_port_ssl(){return m_forward_port_ssl;};
	std::string &pid_file(){return m_pid_file;}
private:
	int m_argc;
	char ** m_argv;
	unsigned short m_local_port;
	std::string m_local_host;
	std::string m_forward_host;
	unsigned short  m_forward_port_ssh;
	unsigned short  m_forward_port_ssl;
	std::string m_pid_file;
};

} /* namespace Configuration */

#endif /* CONFIGURATION_H_ */
