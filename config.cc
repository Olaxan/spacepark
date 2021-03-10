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

static int callback(void*, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
		fprintf(stdout, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

	fprintf(stdout, "\n");
	return EXIT_SUCCESS;
}

int set_pragma(sqlite3*& db, char*& err, const char* pragma, const char* value)
{
	std::ostringstream ss;
	ss << "PRAGMA " << pragma << " = " << value << ";";

	return sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &err);
}

int init_terminals(sqlite3*& db, char*& err)
{
	return sqlite3_exec(db,
			"CREATE TABLE IF NOT EXISTS 'terminals' ("
			"terminal_id INTEGER PRIMARY KEY,"
			"name TEXT UNIQUE);",
			nullptr,
			nullptr,
			&err);
}

int init_pads(sqlite3*& db, char*& err)
{
	// Of course this shouldn't be hardcoded
	// But I have to draw the line somewhere
	return sqlite3_exec(db, 
			"CREATE TABLE IF NOT EXISTS 'pads' ("
			"pad_id INTEGER PRIMARY KEY,"
			"terminal_id INTEGER NOT NULL,"
			"max_weight REAL NOT NULL,"
			"FOREIGN KEY (terminal_id) REFERENCES 'terminals'"
			"ON DELETE CASCADE ON UPDATE CASCADE);",
			nullptr, 
			nullptr,
			&err);
}

int init_ships(sqlite3*& db, char*& err)
{
	return sqlite3_exec(db,
			"CREATE TABLE IF NOT EXISTS 'ships' ("
			"ship_id INTEGER PRIMARY KEY,"
			"license TEXT NOT NULL,"
			"manufacturer TEXT,"
			"weight REAL NOT NULL,"
			"pad_id INTEGER UNIQUE NOT NULL,"
			"FOREIGN KEY (pad_id) REFERENCES 'pads'"
			"ON DELETE CASCADE ON UPDATE CASCADE);",
			nullptr,
			nullptr,
			&err);
}

int add_terminal(sqlite3*& db, char*& err, char*& name)
{
	std::ostringstream ss;
	ss << "INSERT INTO terminals (name) VALUES ('" << name << "');";

	return sqlite3_exec(db, ss.str().c_str(), callback, nullptr, &err);
}

int add_pad(sqlite3*& db, char*& err, int terminal_id, float max_weight)
{
	std::ostringstream ss;
	ss << "INSERT INTO pads (terminal_id, max_weight)"
		"VALUES (" << terminal_id << ", " << max_weight << ");";

	return sqlite3_exec(db, ss.str().c_str(), callback, nullptr, &err);
}

int main(int argc, char* argv[])
{

	Config cfg;
	sqlite3* db;

	fs::path config_path = fs::current_path().append("config.cfg");
	fs::path db_path = fs::current_path().append("park.db");
	
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

	// Open the database -- a new one will be created if none exists so 
	// this shouldn't fail unless an I/O error occurs,
	// or the path leads to a non-empty non-DB file.
	// Either way we can fail here.
	if (sqlite3_open(db_path.c_str(), &db))
	{
		fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	char* err;

	if (set_pragma(db, err, "foreign_keys", "ON"))
	{
		fprintf(stderr, "Failed to enable foreign keys - %s\n", err);
		sqlite3_free(err);
	}

	for (int index = optind; index < argc; index++)
	{
		if (strcmp(argv[index], "default") == 0)
		{
			create_default_config(config_path);
			fprintf(stdout, "Default configuration created at %s\n", config_path.c_str());
		}
		else if (strcmp(argv[index], "init") == 0)
		{
			int errc = 0;
			if (init_terminals(db, err))
			{
				fprintf(stderr, "Faied to init terminals table - %s\n", err);
				sqlite3_free(err);
				errc++;
			}
			if (init_pads(db, err))
			{
				fprintf(stderr, "Failed to init pads table - %s\n", err);
				sqlite3_free(err);
				errc++;
			}
			if (init_ships(db, err))
			{
				fprintf(stderr, "Failed to init ships table - %s\n", err);
				sqlite3_free(err);
				errc++;
			}

			fprintf(stdout, (errc == 0) ? 
					"Database initialized successfully!\n" : "%i error(s) occurred.\n", errc);
		}
		else if (strcmp(argv[index], "add") == 0)
		{
			if (argc <= index + 1)
			{
				// print add usage instructions
				break;
			}

			index++;

			if (strcmp(argv[index], "terminal") == 0)
			{
				if (argc <= index + 1)
				{
					// print add terminal usage instructions
					break;
				}
				
				if (add_terminal(db, err, argv[++index]))
				{
					fprintf(stderr, "Failed to add terminal - %s\n", err);
					sqlite3_free(err);
				}
			}
			else if (strcmp(argv[index], "pad") == 0)
			{
				if (argc <= index + 2)
				{
					// print add terminal usage instructions
					break;
				}

				int terminal_id = atoi(argv[++index]);
				float max_weight = atof(argv[++index]);

				if (add_pad(db, err, terminal_id, max_weight))
				{
					fprintf(stderr, "Failed to add terminal - %s\n", err);
					sqlite3_free(err);
				}
			}
		}
		else 
		{
			fprintf(stderr, "Unknown operation '%s'!\n", argv[index]);
		}

		no_input = false;
	}

	if (no_input)
		print_usage();
	
	sqlite3_close(db);
	return EXIT_SUCCESS;
} 
