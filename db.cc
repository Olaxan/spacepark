#include "db.h"

#include <stdio.h>
#include <cstdlib>
#include <sstream>

int set_pragma(sqlite3*& db, char*& err, const char* pragma, const char* value)
{
	std::ostringstream ss;
	ss << "PRAGMA " << pragma << " = " << value << ";";

	return sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &err);
}
