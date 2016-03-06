#include "bridge.h"

namespace ssh_ssl_proxy
{
   namespace ip = boost::asio::ip;

  void bridge::start(const std::string& upstream_host, unsigned short upstream_port, const unsigned char *buffer)
  {
	 try
	 {
		 upstream_socket_.connect(ip::tcp::endpoint(
			 boost::asio::ip::address::from_string(upstream_host),
			 upstream_port));
		 boost::asio::write(upstream_socket_, boost::asio::buffer(buffer, 6));
	 }
	 catch (const boost::system::system_error &e)
	 {
		 close();
	 }
	 handle_upstream_connect();
  }

  void bridge::handle_upstream_connect()
  {
	upstream_socket_.async_read_some(
		 boost::asio::buffer(upstream_data_,max_data_length),
		 boost::bind(&bridge::handle_upstream_read,
			  shared_from_this(),
			  boost::asio::placeholders::error,
			  boost::asio::placeholders::bytes_transferred));

	downstream_socket_.async_read_some(
		 boost::asio::buffer(downstream_data_,max_data_length),
		 boost::bind(&bridge::handle_downstream_read,
			  shared_from_this(),
			  boost::asio::placeholders::error,
			  boost::asio::placeholders::bytes_transferred));
  }

  void bridge::handle_downstream_write(const boost::system::error_code& error)
  {
	 if (!error)
	 {
		upstream_socket_.async_read_some(
			 boost::asio::buffer(upstream_data_,max_data_length),
			 boost::bind(&bridge::handle_upstream_read,
				  shared_from_this(),
				  boost::asio::placeholders::error,
				  boost::asio::placeholders::bytes_transferred));
	 }
	 else
		close();
  }

  void bridge::handle_downstream_read(const boost::system::error_code& error,
                                  const size_t& bytes_transferred)
  {
	 if (!error)
	 {
		async_write(upstream_socket_,
			  boost::asio::buffer(downstream_data_,bytes_transferred),
			  boost::bind(&bridge::handle_upstream_write,
					shared_from_this(),
					boost::asio::placeholders::error));
	 }
	 else
		close();
  }

  void bridge::handle_upstream_write(const boost::system::error_code& error)
  {
	 if (!error)
	 {
		downstream_socket_.async_read_some(
			 boost::asio::buffer(downstream_data_,max_data_length),
			 boost::bind(&bridge::handle_downstream_read,
				  shared_from_this(),
				  boost::asio::placeholders::error,
				  boost::asio::placeholders::bytes_transferred));
	 }
	 else
		close();
  }

  void bridge::handle_upstream_read(const boost::system::error_code& error,
                                const size_t& bytes_transferred)
  {
	 if (!error)
	 {
		async_write(downstream_socket_,
			 boost::asio::buffer(upstream_data_,bytes_transferred),
			 boost::bind(&bridge::handle_downstream_write,
				  shared_from_this(),
				  boost::asio::placeholders::error));
	 }
	 else
		close();
  }

  void bridge::close()
  {
	 boost::mutex::scoped_lock lock(mutex_);
	 if (downstream_socket_.is_open())
		downstream_socket_.close();
	 if (upstream_socket_.is_open())
		upstream_socket_.close();
  }

	bool bridge::acceptor::accept_connections()
	{
		try
		{
		   session_ = boost::shared_ptr<bridge>(new bridge(io_service_));
		   acceptor_.async_accept(session_->downstream_socket(),
				boost::bind(&acceptor::handle_accept,
					 this,
					 boost::asio::placeholders::error));
		}
		catch(std::exception& e)
		{
		   std::cerr << "acceptor exception: " << e.what() << std::endl;
		   return false;
		}
		return true;
	}

    bool bridge::acceptor::isSSL(const unsigned char * buffers)
	{
		if (buffers[0] & 0x80) // SSLv2 maybe
		 {
		   int length = (int(buffers[0] & 0x7f) << 8) + buffers[1];
		   if (length > 9) // likely SSLv2
		   {
			 if (buffers[2] == 0x01) // definitely SSLv2
			 {
			   std::cout << "SSL v2.0 Handshake detected" << std::endl;
			   return true;
			 }
		   }
		 }
		 else if (buffers[0] == 0x16) // SSLv3/TLSv1.x maybe
		 {
		   if (buffers[1] == 3) // very likely SSLv3/TLSv1.x handshake
		   {
			 if (buffers[2]) // TLS v1.x
			 {
			   std::cout << "TLS v1.x Handshake detected" << std::endl;
			 }
			 else // SSL v3.0
			 {
			   std::cout << "SSL v3.0 Handshake detected" << std::endl;
			 }
			 return true;
		   }
		 }
		std::cout << "It is SSH or something else" << std::endl;
		return false;
	}

	void bridge::acceptor::handle_accept(const boost::system::error_code& error)
	{
		if (!error)
		{
		   // read 6 bytes from downstream to distinguish if it is ssl or ssh
		   unsigned char buffer[6];
		   try
		   {
			   boost::asio::read(session_->downstream_socket(), boost::asio::buffer(buffer));
			   upstream_port_ = isSSL(buffer) ?  upstream_port_ssl_ : upstream_port_ssh_;
		   }
		   catch (const boost::system::system_error &e)
		   {
			   std::cerr << "Error: " << e.what() << std::endl;
			   return;
		   }

		   session_->start(upstream_host_,upstream_port_, buffer);
		   if (!accept_connections())
		   {
			  std::cerr << "Failure during call to accept." << std::endl;
		   }
		}
		else
		{
		   std::cerr << "Error: " << error.message() << std::endl;
		}
	}
}
