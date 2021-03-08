#pragma once

// External
#include <libconfig.h++>
#include <sqlite3.h>
#include <asio.hpp>

// STL
#include <filesystem>

namespace fs = std::filesystem;

class parking_server
{
	public:

	parking_server(libconfig::Config& cfg, fs::path db_path);
	~parking_server();

	void start();
	void stop();

	private:

	asio::ip::tcp::endpoint _endpoint;
	libconfig::Config& _config;
	sqlite3* _db;
	bool _open;
};
