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
			int dock;
			if ((dock = server.get_free_dock(0)) > 0)
				fprintf(stdout, "Found free dock: %d\n", dock);
			else
				fprintf(stderr, "No free dock found!");
		}
	}

	sqlite3_close(db);
	return EXIT_SUCCESS;
} 
