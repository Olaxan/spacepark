#pragma once

#include "asio/error_code.hpp"
#include <asio.hpp>

#include <memory>

constexpr int max_data_size = 256;

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

		void handle_message(const asio::error_code& err, std::size_t bytes);
		void begin();

	private:

		char* _data[max_data_size];
		asio::ip::tcp::socket _socket;

};
