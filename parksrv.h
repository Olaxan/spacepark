#pragma once

// External
#include "asio/io_service.hpp"
#include <memory>
#include <sqlite3.h>
#include <asio.hpp>

#include "session.h"

class parking_server
{
	public:

		parking_server(sqlite3*& db, int port);
		~parking_server();

		void open();
		void close();

	private:

		asio::io_service _ios;
		asio::ip::tcp::acceptor _acceptor;
		sqlite3*& _db;

		void start_accept();
		void handle_accept(std::shared_ptr<parking_session> session, const asio::error_code& err);
};
