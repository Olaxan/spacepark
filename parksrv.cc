#include "parksrv.h"
#include "asio/error_code.hpp"

#include <functional>
#include <memory>

using asio::ip::tcp;

parking_server::parking_server(sqlite3*& db, int port)
	: _acceptor(_ios, tcp::endpoint(tcp::v4(), port)), _db(db)
{
}

parking_server::~parking_server()
{
}

void parking_server::handle_accept(std::shared_ptr<parking_session> session, const asio::error_code& err)
{
	if (!err)
		session->begin();

	start_accept();
}

void parking_server::start_accept()
{
	std::shared_ptr<parking_session> session = std::make_shared<parking_session>(_ios);	

	_acceptor.async_accept(
			session->socket(),
			std::bind(&parking_server::handle_accept, 
				this, 
				session, 
				std::placeholders::_1));

}

void parking_server::open()
{
	fprintf(stdout, "Server is running on port %i...\n", _acceptor.local_endpoint().port());
	start_accept();
}

