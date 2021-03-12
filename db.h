/*
 * This file is part of SPACEPARK.
 *
 * Developed for the VISMA graduate program code challenge.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * If issues occur, contact me on fredrik.lind.96@gmail.com
 *
 */

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
