#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

// STL
#include <filesystem>

// Externals
#include <sqlite3.h>
#include <libconfig.h++>

namespace fs = std::filesystem;
using namespace libconfig;

void print_usage()
{
	printf("SPACEPARK configuration utility\n"
			"Copyleft 2142 - Tonto Turbo AB\n\n"
			"Use this utility to configure the main application and setup a database.\n\n"
			"usage: \tspacepark-config [-h] [-c <path>] [-d <path>]\n"
		       "options:\n"
		       "\t-h:\t\tShows this help.\n"
			   "\t-c <path>:\tSpecify the configuration path.\n"
			   "\t-d <path>:\tSpecify the database file path.\n\n"
	      );
}

void create_default_config(fs::path& stream)
{
	Config cfg;
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
	bool no_input = true;

	opterr = 0;

	while ((c = getopt(argc, argv, "hc:d:")) != -1)
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
				if (optopt == 'c' || optopt == 'c')
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

	for (int index = optind; index < argc; index++)
	{
		if (strcmp(argv[index], "default") == 0)
		{
			create_default_config(config_path);
			fprintf(stdout, "Default configuration created at %s\n", config_path.c_str());
			return EXIT_SUCCESS;
		}

		if (strcmp(argv[index], "init") == 0)
		{
			fprintf(stdout, "Creating new database...\n");
			return EXIT_SUCCESS;
		}

		if (strcmp(argv[index], "delete") == 0)
		{
			fprintf(stdout, "Deleting database...\n");
		}
		else
		{
			fprintf(stderr, "Unknown operation '%s'!\n", argv[index]);
			return EXIT_FAILURE;
		}

		no_input = false;
	}

	if (no_input)
		print_usage();
	
	return EXIT_SUCCESS;
} 
