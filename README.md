# SPACEPARK 

A headless parking system server running SQLite, for managing small to medium spaceship docks.
This is my solution to the VISMA Graduate Program code assignment.
It is not fully complete.

## Limitations
More than I can count. Most invalid database operations fail without helpful error codes or messages,
	 so the user won't get a clue what's going wrong. This is partly because the functions performing
	 these operations are server-internal (and as such not meant to be directly interacted with),
	 and partly because I don't have the time to add error codes.

## Usage

Run the application to generate a default configuration file, then edit it to your liking.
Re-run the application when everything looks good.

Run with -h flag for usage instructions.

## Build instructions

The application is dependent on GNU extensions, such as asprintf,
	as well as POSIX functions such as getopt.
	These can be replaced with relative ease, but in its current state the application
	is Linux only.

Linux:

Build project files with CMake and compile on GCC 8.2+
(required for std::filesystem).

Remember to copy compile-commands.json to the root directory
if you want YCM syntax highlighting.

The project should include all external dependencies, 
but if you have problems with missing libraries, install (with your favourite package manager):
* libconfig 

## Dependencies

* sqlite3 
	https://www.sqlite.org/index.html
* libconfig
	https://hyperrealm.github.io/libconfig/
