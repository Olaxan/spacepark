#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <ctype.h>

// STL
#include <filesystem>

// Externals
#include <sqlite3.h>
#include <libconfig.h++>

namespace fs = std::filesystem;
using namespace libconfig;

void print_usage()
{
	printf("SPACEPARK server utility\n"
			"Copyleft 2142 Tonto Turbo AB\n\n"
			"Launch a headless SPACEPARK server, with an optional CLI\n\n"
			"usage: \tspacepark [-h] [-s] [-w <wxh>] [-o <path>]\n"
		       "\t[-r <rays>] [-n <name>] [-x <seed>] [-b <bounces>]\n"
		       "options:\n"
		       "\t-h:\t\tShows this help.\n"
	      );
}

void create_default_config(Config& cfg, fs::path& stream)
{
	Setting &root = cfg.getRoot();
	root.add("db_path", Setting::TypeString) = fs::current_path().append("park.db");
	cfg.writeFile(stream.c_str());
}

int main(int argc, char* argv[])
{

	Config cfg;

	fs::path config_path = fs::current_path().append("config.cfg");
	fs::path db_path;
	
	int c;
	int argval;

	opterr = 0;

	while ((c = getopt (argc, argv, "hsw:o:r:n:x:b:t:m:")) != -1)
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
			case '?':
				if (optopt == 'c')
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
				db_path = fs::current_path().append("park.db");
			}
		}
	}
	else
	{
		fprintf(stderr, 
				"The config file could not be located.\n"
				"Config path: %s\n\n"
				"A default has been created in this location.\n"
				"Please make sure it is correct, then re-run the utility.", 
				config_path.c_str());

		create_default_config(cfg, config_path);

		return EXIT_FAILURE;
	}

	// We have a config - prepare to open DB
	sqlite3 *db;

	if (!fs::exists(db_path))
	{
		fprintf(stderr, 
				"The database file could not be located.\n"
				"DB Path: %s\n\n"
				"A new database will be opened in this location.\n",
				db_path.c_str());
	}

	if (sqlite3_open(db_path.c_str(), &db))
	{
		fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return EXIT_FAILURE;
	}

	fprintf(stdout, "SPACEPARK server running...\n");
	sqlite3_close(db);
	return EXIT_SUCCESS;
} 
