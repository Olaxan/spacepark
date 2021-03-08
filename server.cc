#include "server.h"

parking_server::parking_server(libconfig::Config& cfg, fs::path db_path)
	: _config(cfg), _open(false)
{
	if (!fs::exists(db_path))
	{
		fprintf(stderr, 
				"The database file could not be located.\n"
				"DB Path: %s\n\n"
				"A new database will be opened in this location.\n",
				db_path.c_str());
	}

	if (sqlite3_open(db_path.c_str(), &_db))
	{
		fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(_db));
		sqlite3_close(_db);
		return;
	}

	_open = true;
}

parking_server::~parking_server()
{
	if (_open)
		sqlite3_close(_db);
}

void parking_server::start()
{

}
