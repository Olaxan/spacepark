#include "session.h"

#include <asio.hpp>

#include "asio/read.hpp"
#include "protocol.h"

using asio::ip::tcp;

void parking_session::handle_message(const asio::error_code& err, std::size_t bytes)
{
	if (err)
		fprintf(stderr, "Recieve error - %s\n", err.message().c_str());
	else
	{
		msg_head head;
		memcpy(&head, _data, bytes);
		fprintf(stdout, "MESSAGE: %u, %u, %u, %d\n", head.length, head.seq_no, head.id, (int)head.type);
	}
}

void parking_session::begin()
{
	asio::async_read(_socket, 
			asio::buffer(_data, sizeof(msg_head)),
			std::bind(&parking_session::handle_message, 
				this, 
				std::placeholders::_1,
				std::placeholders::_2));
}
