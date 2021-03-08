# SPACEPARK 

A headless parking system server running SQLite, for managing small to medium spaceship docks.
This is my solution to the VISMA Graduate Program code assignment.

## Usage

Run the application to generate a default configuration file, then edit it to your liking.
Re-run the application when everything looks good.

Run with -h flag for usage instructions.

## Build instructions

Windows:

* You can either generate visual studio solution by using CMake, or just open the folder within VSCode and build/run.

Linux:

Build project files with CMake and compile on GCC 8.2+
(required for std::filesystem)

The project should include all external dependencies, 
but if you have problems with missing libraries, install (with your favourite package manager):
* libconfig 

## Dependencies

* sqlite3 
	https://www.sqlite.org/index.html
* libconfig
	https://hyperrealm.github.io/libconfig/
