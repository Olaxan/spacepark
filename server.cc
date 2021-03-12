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
 */

#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

// STL
#include <filesystem>

// Externals
#include <sqlite3.h>
#include <libconfig.h++>

// Relative
#include "parksrv.h"
#include "db.h"

namespace fs = std::filesystem;
using namespace libconfig;

void print_usage()
{
	printf("SPACEPARK server utility\n"
			"\nSpace-copyright 2142 - Tonto Turbo AB\n"
			"\nUse this utility to launch a spacepark server, or invoke one-time commands.\n"
			"\nusage:\tspacepark-server [-h] [-c <path>] [-p <begin-end>]"
			"\n\t[-d <path>] <command> [<args>]"
		    "\noptions:"
		    "\n\t-h:\t\tShows this help"
			"\n\t-c <path>:\tSpecify the configuration path"
			"\n\t-d <path>:\tSpecify the database file path\n"
			"\n\t-p <begin-end>:\tSpecify the port range\n"
			"\ncommands:\n"
			"\n\topen\t\tOpen the server"
			"\n\tdock\t\tDock a ship at a specified pad"
			"\n\tundock\t\tUndock a ship from a specified pad"
			"\n\tseconds\t\tGet number of seconds docked at pad"
			"\n\tfee\t\tGet current parking fee for ship docked at pad"
			"\n\tdump\t\tDump a DB table into stdout"
			"\n"
	      );
}

static int dump_callback(void*, int argc, char** argv, char**)
{
	for (int i = 0; i < argc; i++)
		fprintf(stdout, "%s\t", argv[i] ? argv[i] : "NULL");

	fprintf(stdout, "\n");
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{

	Config cfg;

	fs::path config_path = fs::current_path().append("config.cfg");
	fs::path db_path;
	
	int port_begin = 0;
	int port_end = 0;

	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "hc:d:p:")) != -1)
	{
		switch (c)
		{
			case 'h':
				print_usage();
				return EXIT_SUCCESS;
			case 'c':
				config_path = optarg;
				break;
			case 'd':
				db_path = optarg;
				break;
			case 'p':
				if (sscanf(optarg, "%d-%d", &port_begin, &port_end) != 2)
				{
					fprintf(stderr, "Specify a valid port range in the format 'A-B'.\n");
					return EXIT_FAILURE;
				}
				break;
			case '?':
				if (optopt == 'c' || optopt == 'p')
					fprintf (stderr, "Option '-%c' requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n",optopt);
				return EXIT_FAILURE;
			default:
				return EXIT_FAILURE;
		}
	}
	
	if (fs::exists(config_path))
	{
		try 
		{ 
			cfg.readFile(config_path.c_str());
		}
		catch(const FileIOException &fioex)
		{
			fprintf(stderr, "I/O error while reading configuration.");

			return EXIT_FAILURE;
		}

		// Query for DB path from config if not user specified
		if (db_path.empty())
		{
			try
			{
				db_path = cfg.lookup("db_path");
			}
			catch (const SettingNotFoundException &nfex)
			{
				fprintf(stderr, "No database path was specified");

				return EXIT_FAILURE;
			}
		}

		if (port_begin == 0)
		{
			try
			{
				port_begin = cfg.lookup("port_begin");
				port_end = cfg.lookup("port_end");
			}
			catch (const SettingNotFoundException &nfex)
			{
				fprintf(stderr, "No port range was specified or configured.\n");
				return EXIT_FAILURE;
			}
		}
	}
	else
	{
		fprintf(stderr, 
				"The config file could not be located.\n"
				"Config path: %s\n\n"
				"Please run spacepark-config to create one, "
				"or run the utility with the -c switch to specify a config path.\n",
				config_path.c_str());

		return EXIT_FAILURE;
	}

	if (!fs::exists(db_path))
	{
		fprintf(stderr, 
				"The database file could not be located.\n"
				"DB Path: %s\n\n"
				"Please run spacepark-config to create one, "
				"or run the utility with the -d switch to specify a database path.\n",
				db_path.c_str());

		return EXIT_FAILURE;
	}

	sqlite3* db;

	if (sqlite3_open(db_path.c_str(), &db))
	{
		fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return EXIT_FAILURE;
	}

	char* err;

	if (set_pragma(db, err, "foreign_keys", "ON"))
	{
		fprintf(stderr, "Failed to enable foreign keys constraint.\n"
				"Make sure SQLite is compiled with neither:\n"
				" - SQLITE_OMIT_FOREIGN_KEY\n"
				" - SQLITE_OMIT_TRIGGER\n\n"
				"Error: %s\n", err);

		sqlite3_free(err);
		sqlite3_close(db);

		return EXIT_FAILURE;
	}

	parking_server server(db);

	for (int index = optind; index < argc; index++)
	{
		if (strcmp(argv[index], "open") == 0)
		{
			if (server.open(port_begin, port_end))
				fprintf(stderr, "Server exited with an error.\n");
			else
				fprintf(stdout, "Server exited cleanly.\n");

			break;
		}
		else if (strcmp(argv[index], "free") == 0)
		{
			if (argc > index + 1)
			{
				int dock = atoi(argv[++index]);
				bool is_free = server.dock_is_free(dock);
				fprintf(stdout, "Dock %d is %s.\n", dock, is_free ? "free" : "occupied");
			}
			else
			{
				int dock;
				if ((dock = server.get_free_dock(0)) > 0)
					fprintf(stdout, "Found free dock: %d\n", dock);
				else
					fprintf(stderr, "No free dock found!");
			}

			break;
		}
		else if (strcmp(argv[index], "dock") == 0)
		{
			if (argc <= index + 3)
			{
				fprintf(stderr, "Usage: spacepark-dock <PAD ID> <WEIGHT> <LICENSE>\n");
				break;
			}

			int id = atoi(argv[++index]);
			float weight = atof(argv[++index]);
			int rc = server.dock_ship(id, weight, argv[++index]);

			if (rc == SQLITE_OK)
				fprintf(stdout, "Docked successfully.\n");
			else
				fprintf(stderr, "Error %d occurred during docking.\n", rc);

			break;
		}
		else if (strcmp(argv[index], "undock") == 0)
		{
			if (argc <= index + 1)
			{
				fprintf(stderr, "Usage: spacepark-server undock <PAD ID>\n");
				break;
			}

			int id = atoi(argv[++index]);
			int fee = server.get_fee(id);
			int rc = server.undock_ship(id);

			if (rc == SQLITE_OK)
				fprintf(stdout, "Undocked successfully, parking fee is %d credits.\n", fee);
			else
				fprintf(stderr, "Failed to undock.\n");

			break;
		}
		else if (strcmp(argv[index], "seconds") == 0)
		{
			if (argc <= index + 1)
			{
				fprintf(stderr, "Usage: spacepark-server seconds <PAD ID>\n");
				break;
			}

			int id = atoi(argv[++index]);

			fprintf(stdout, "Ship at pad %d has been docked for %d seconds.\n",
					id, server.get_seconds_docked(id));

			break;
		}
		else if (strcmp(argv[index], "fee") == 0)
		{
			if (argc <= index + 1)
			{
				fprintf(stderr, "Usage: spacepark-server fee <PAD ID>\n");
				break;
			}

			int id = atoi(argv[++index]);
			int fee = server.get_fee(id);

			if (fee == -1)
				fprintf(stderr, "No ship is docked at bay %d.\n", id);
			else
				fprintf(stdout, "Ship at pad %d has a parking fee of %d credits.\n",
						id, fee);

			break;
		}
		else if (strcmp(argv[index], "dump") == 0)
		{
			if (argc <= index + 1)
			{
				fprintf(stderr, "Usage: spacepark-server dump <TABLE>\n");
				break;
			}

			char* statement;
			char* err;
			int c, rc;

			// This is for debugging primarly!
			if ((c = asprintf(&statement, "SELECT * FROM %s;", argv[++index])) > 0)
			{
				sqlite3_stmt* s;
				int src = sqlite3_prepare_v2(db, statement, c, &s, nullptr);

				if (src == SQLITE_OK)
				{
					for (int i = 0; i < sqlite3_column_count(s); i++)
						fprintf(stdout, "%s\t", sqlite3_column_name(s, i));

					fprintf(stdout, "\n ------------------------------------------------- \n");

					// It's stupid not to use our prepared statement, I know,
					// but this is RUSHED code!!
					if ((rc = sqlite3_exec(db, statement, dump_callback, nullptr, &err)) != SQLITE_OK)
					{
						fprintf(stderr, "Query error %d - %s\nQuery: %s", rc, err, statement);
						sqlite3_free(err);
					}

				}

				sqlite3_finalize(s);
				free(statement);
			}

			break;
		}
		else print_usage();
	}

	// Ensure we always close the DB connection.
	// Make sure we reach this point or close it explicitly.
	sqlite3_close(db);
	return EXIT_SUCCESS;

} 
