/*
 * Configuration.h
 *
 *  Created on: Dec 20, 2015
 *      Author: dafe
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
private:
	int m_argc;
	char ** m_argv;
	unsigned short m_local_port;
	std::string m_local_host;
	std::string m_forward_host;
	unsigned short  m_forward_port_ssh;
	unsigned short  m_forward_port_ssl;
};

} /* namespace Configuration */

#endif /* CONFIGURATION_H_ */
