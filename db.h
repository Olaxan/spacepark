#pragma once

#include <sqlite3.h>

int set_pragma(sqlite3*& db, char*& err, const char* pragma, const char* value);
