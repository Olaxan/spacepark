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

namespace fs = std::filesystem;
using namespace libconfig;

void print_usage()
{
	printf("SPACEPARK server utility\n"
			"Copyleft 2142 - Tonto Turbo AB\n\n"
			"Launch a headless SPACEPARK server, with an optional CLI\n\n"
			"usage: \tspacepark [-h] [-s] [-w <wxh>] [-o <path>]\n"
		       "\t[-r <rays>] [-n <name>] [-x <seed>] [-b <bounces>]\n"
		       "options:\n"
		       "\t-h:\t\tShows this help.\n"
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
	
	int port = 0;
	
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
				port = atoi(optarg);
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

		if (port == 0)
		{
			try
			{
				port = cfg.lookup("port");
			}
			catch (const SettingNotFoundException &nfex)
			{ } // Do nothing -- ASIO autoconfigures a port when given a 0
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

	parking_server server(db);

	for (int index = optind; index < argc; index++)
	{
		if (strcmp(argv[index], "open") == 0)
		{
			if (server.open(port, port + 64))
				fprintf(stderr, "Server exited with an error.\n");
			else
				fprintf(stdout, "Server exited cleanly.\n");
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
		}
		else if (strcmp(argv[index], "dock") == 0)
		{
			if (argc <= index + 3)
				break;

			int id = atoi(argv[++index]);
			float weight = atof(argv[++index]);
			int rc = server.dock_ship(id, weight, argv[++index]);

			if (rc == SQLITE_OK)
				fprintf(stdout, "Docked successfully.\n");
			else
				fprintf(stderr, "Error %d occurred during docking.\n", rc);
		}
		else if (strcmp(argv[index], "undock") == 0)
		{
			if (argc <= index + 1)
				break;

			int id = atoi(argv[++index]);
			int rc = server.undock_ship(id);

			if (rc == SQLITE_OK)
				fprintf(stdout, "Undocked successfully.\n");
			else
				fprintf(stderr, "Failed to undock.\n");
		}
		else if (strcmp(argv[index], "dump") == 0)
		{
			if (argc <= index + 1)
				break;

			char* statement;
			char* err;
			int c, rc;

			if ((c = asprintf(&statement, "SELECT * FROM %s;", argv[++index])) > 0)
			{
				sqlite3_stmt* s;
				int src = sqlite3_prepare_v2(db, statement, c, &s, nullptr);

				if (src == SQLITE_OK)
				{
					for (int i = 0; i < sqlite3_column_count(s); i++)
						fprintf(stdout, "%s\t", sqlite3_column_name(s, i));

					fprintf(stdout, "\n");

					// It's stupid not to use our prepared statement, I know,
					// but this is RUSHED code!!
					if ((rc = sqlite3_exec(db, statement, dump_callback, nullptr, &err)) != SQLITE_OK)
					{
						fprintf(stderr, "Query error %d - %s\nQuery: %s", rc, err, statement);
						sqlite3_free(err);
					}

					sqlite3_finalize(s);
				}


				free(statement);
			}
		}
	}

	sqlite3_close(db);
	return EXIT_SUCCESS;
} 
