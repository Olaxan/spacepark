#pragma once

#include <sqlite3.h>

/**
 * Set a PRAGMA statement in the open DB.
 *
 * RISK OF MEMORY LEAK:
 * Note that the err pointer will be dynamically allocated by
 * SQLite upon error! If an error occurs, i.e. the function
 * returns non-zero, it will need to be freed using
 * sqlite3_free(err) when no longer needed.
 *
 * @param db The SQLite DB connection, expected to be open.
 * @param err The error char string returned by SQLite.
 * @param pragma The pragma name to set.
 * @param value The value of the pragma to set.
 * @return A SQLite response code.
 */
int set_pragma(sqlite3*& db, char*& err, const char* pragma, const char* value);
