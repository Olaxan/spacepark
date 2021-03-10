#pragma once

#include <asio.hpp>

#include <memory>

class parking_session
: std::enable_shared_from_this<parking_session>
{
	public:

		parking_session(asio::io_service& service)
			: _socket(service) { }

		asio::ip::tcp::socket& socket()
		{
			return _socket;
		}

		void begin();

	private:

		asio::ip::tcp::socket _socket;

};
