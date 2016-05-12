/* This file is part of ssh_ssl_proxy.

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
//
// tcpproxy_server.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2007 Arash Partow (http://www.partow.net)
// URL: http://www.partow.net/programming/tcpproxy/index.html
//
// Distributed under the Boost Software License, Version 1.0.
//
//
// Description
// ~~~~~~~~~~~
// The objective of the TCP proxy server is to act as an intermediary
// in order to 'forward' TCP based connections from external clients
// onto a singular remote server.
// The communication flow in the direction from the client to the proxy
// to the server is called the upstream flow, and the communication flow
// in the direction from the server to the proxy to the client is called
// the downstream flow. Furthermore the up and down stream connections
// are consolidated into a single concept known as a bridge.
// In the event either the downstream or upstream end points disconnect,
// the proxy server will proceed to disconnect the other end point
// and eventually destroy the associated bridge.
//
// The following is a flow and structural diagram depicting the
// various elements (proxy, server and client) and how they connect
// and interact with each other.
//
//                                    ---> upstream --->           +---------------+
//                                                     +---->------>               |
//                               +-----------+         |           | Remote Server |
//                     +--------->          [x]--->----+  +---<---[x]              |
//                     |         | TCP Proxy |            |        +---------------+
// +-----------+       |  +--<--[x] Server   <-----<------+
// |          [x]--->--+  |      +-----------+
// |  Client   |          |
// |           <-----<----+
// +-----------+
//                <--- downstream <---
//
/* 6.3.2016 dafe@dafe.net added detection of ssl and ssh connection
 -> bool isSSL(const unsigned char * buffers) is based on
 http://cboard.cprogramming.com/networking-device-communication/166336-detecting-ssl-tls-client-handshake.html
 project was split to header and cpp file.
 */

#include "ssh_ssl_proxy.h"
#include "boost/optional/optional.hpp"

namespace ssh_ssl_proxy {
namespace ip = boost::asio::ip;

class bridge: public boost::enable_shared_from_this<bridge> {
public:

	typedef ip::tcp::socket socket_type;
	typedef boost::shared_ptr<bridge> ptr_type;

	bridge(boost::asio::io_service& ios) :
			downstream_socket_(ios), upstream_socket_(ios) {
	}

	socket_type& downstream_socket() {
		return downstream_socket_;
	}

	socket_type& upstream_socket() {
		return upstream_socket_;
	}

	void start(const std::string& upstream_host, unsigned short upstream_port,
			const unsigned char *buffer);
	void handle_upstream_connect();

private:

	void handle_downstream_write(const boost::system::error_code& error);
	void handle_downstream_read(const boost::system::error_code& error,
			const size_t& bytes_transferred);
	void handle_upstream_write(const boost::system::error_code& error);
	void handle_upstream_read(const boost::system::error_code& error,
			const size_t& bytes_transferred);
	void close();

	socket_type downstream_socket_;
	socket_type upstream_socket_;

	enum {
		max_data_length = 8192
	}; //8KB
	unsigned char downstream_data_[max_data_length];
	unsigned char upstream_data_[max_data_length];

	boost::mutex mutex_;

public:

	class acceptor {
	public:

		acceptor(boost::asio::io_service& io_service,
				const std::string& local_host, unsigned short local_port,
				const std::string& upstream_host,
				unsigned short upstream_port_ssh,
				unsigned short upstream_port_ssl) :
				io_service_(io_service), localhost_address(
						boost::asio::ip::address_v4::from_string(local_host)), acceptor_(
						io_service_,
						ip::tcp::endpoint(localhost_address, local_port)), upstream_port_ssh_(
						upstream_port_ssh), upstream_port_ssl_(
						upstream_port_ssl), upstream_port_(0), upstream_host_(
						upstream_host) {
		}

		bool accept_connections();

	private:
		bool isSSL(const unsigned char * buffers);
		void handle_accept(const boost::system::error_code& error);

		boost::asio::io_service& io_service_;
		ip::address_v4 localhost_address;
		ip::tcp::acceptor acceptor_;
		ptr_type session_;
		unsigned short upstream_port_ssh_;
		unsigned short upstream_port_ssl_;
		unsigned short upstream_port_;
		std::string upstream_host_;
	};

};
}
