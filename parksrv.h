#pragma once

// External
#include <libconfig.h++>
#include <sqlite3.h>
#include <asio.hpp>

class parking_server
{
	public:

	parking_server(libconfig::Config& cfg, sqlite3*& db);
	~parking_server();

	void start();
	void stop();

	private:

	asio::ip::tcp::endpoint _endpoint;
	libconfig::Config& _config;
	sqlite3*& _db;
};
