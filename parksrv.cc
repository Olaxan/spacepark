#include "parksrv.h"

parking_server::parking_server(libconfig::Config& cfg, sqlite3*& db)
	: _config(cfg), _db(db)
{
}

parking_server::~parking_server()
{
}

void parking_server::start()
{

}
