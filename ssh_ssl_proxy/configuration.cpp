/*
  configuration.cpp

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

 Config file example:
 	 localhost=127.0.0.1
     localport=3333
 	 forward_host=192.168.2.13
 	 forward_port_ssh=22
 	 forward_port_ssl=443

 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <exception>

#include "configuration.h"

namespace ssh_ssl_proxy {

configuration::configuration(int argc, char* argv[]) :
		m_argc(argc), m_argv(argv), m_local_port(0), m_forward_port_ssh(22), m_forward_port_ssl(
				443) {
}

configuration::~configuration() {
}

void configuration::load() {
	if (m_argc == 2) {
		// loading config file
		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini(m_argv[1], pt);
		m_local_port = static_cast<unsigned short>(::atoi(
				pt.get<std::string>("localport").c_str()));
		m_local_host = pt.get<std::string>("localhost");
		m_forward_host = pt.get<std::string>("forward_host");
		m_forward_port_ssh = static_cast<unsigned short>(::atoi(
				pt.get<std::string>("forward_port_ssh").c_str()));
		m_forward_port_ssl = static_cast<unsigned short>(::atoi(
				pt.get<std::string>("forward_port_ssl").c_str()));
		m_pid_file = pt.get<std::string>("pidfile");
		return;
	}
	if (m_argc == 4) {
		// loading properties from command line
		m_local_port = static_cast<unsigned short>(::atoi(m_argv[2]));
		m_local_host = m_argv[1];
		m_forward_host = m_argv[3];
		return;
	}
	show_usage();
	throw std::runtime_error("wrong parameters");
}

void configuration::show_usage() {
	std::cerr
			<< "usage: ssh_ssl_proxy <local host ip> <local port> <forward host ip>"
			<< std::endl;
	std::cerr << "     : ssh_ssl_proxy config.txt" << std::endl;
}
}/* namespace ssh_ssl_proxy */
