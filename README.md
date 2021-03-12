# SPACEPARK 

A (partly) headless parking system server running SQLite, for managing small to medium spaceship docks.
This is my solution to the VISMA Graduate Program code assignment.
It is not fully complete, and contains several bugs 
  -- some would probably be critical for spaceflight purposes!!

## Usage

### Configuration

This repository includes a pre-setup database and configuration file (park.db, config.cfg), in the demo folder.
Place them in same directory as the binary, or refer to them with the -d and -c switches.
In order to setup your own, follow these steps:
1. Remove any old database or config files.
1. Run `spacepark-config default` in order to generate a config file,
and edit it to your likings.
1. Run `spacepark-config init` to initialize an empty database, at the location specified either in the config file or by the -d switch.
1. Run `spacepark-config add terminal <NAME 1> <NAME 2> <NAME 3> ...` to add any number of terminals (floors).
1. Run `spacepark config add pad <TERMINAL ID> <MAX WEIGHT> <COUNT>` to add landing pads to the specified terminal. Note that the terminal ID is equal to its row ID in the database, not the name. You can find the ID:s for existing terminals by running `spacepark-server dump terminals` (this will be fixed in the future).
1. The server is now ready to use!

### Running the server

The server can be launched with `spacepark-server open`, which will start a TCP-IPv4* server listening
to the port range specified in the configuration, or by running the application with the -p switch.

Close the server by invoking SIGINT. It's not graceful! Hopefully it's not doing any DB operations when you do that (although SQLite should handle an interrupted transaction fairly well).

_*) Hopefully IPv4 is still around when we have readily available commercial spaceflight._

### Invoking local commands

All operations can be performed one-time by running the appropriate server command. In its current state, this is probably the preferred way to interact with the application.
* Run `spacepark-server free` to find a free spaceship dock (with no weight restriction specified).
* Run `spacepark-server free <DOCK ID>` to check an existing dock for occupancy.
* Run `spacepark-server dock <DOCK ID> <WEIGHT> <LICENSE>` to register a ship to a landing pad.
* Run `spacepark-server undock <DOCK ID>` to register a ship undocking from a landing pad.
* Run `spacepark-server seconds <DOCK ID>` to query the number of seconds a ship has been docked at a specified pad.
* Run `spacepark-server fee <DOCK ID>` to query the current parking fee of a ship parked at a specified dock -- note that these fees may vary depending on the dock (currently there is no way to specifiy these fees using the application, it must be done with a database query).
* Run `spacepark-server dump <TABLE>` to get a printout of all entries in the specified table. Currently named tables include *ships*, *pads*, *terminals* and *docking-log*.

### Using the client

What client?

## Build instructions

The application is dependent on GNU extensions, such as asprintf 
as well as POSIX functions such as getopt.
These can be replaced with relative ease, but in its current state the application 
is Linux only (but who'd want to run a server on Windows anyway?).

### Linux:

Build project files with CMake and compile on GCC 8.2+
(required for std::filesystem, can easily be removed which would lower
 the compiler requirement significantly).
Remember to copy compile-commands.json to the root directory 
if you want YCM syntax highlighting.

The project should include all external dependencies, 
but if you have problems with missing libraries, install (with your favourite package manager):
* libconfig 
* sqlite3

## Limitations

Quite a few!

* The built-in help is lacking and some commands fail without reporting why.
* Most invalid database operations fail without helpful error codes or messages, 
so the user won't get a clue what's going wrong. This is partly because the functions performing 
these operations are server-internal (and as such not meant to be directly interacted with), 
and partly because I don't have the time to fix this issue.
* The code could be better commented. Some parts look a little insane?
* There is a fair bit of code reuse in the SQL queries inside *parksrv.cc*. Perhaps it would be possible to write a wrapper function for dispatching those.
* Frequently used SQL statements should probably be prepared in advance, and not executed as hardcoded strings every time.
* The TCP server is untested and lacks a corresponding client. 
I'm also aware that sending structs over TCP is bad practice and vulnerable to problems with endianness, packing, and compiler trickery. It was mostly just for fun -- the messaging should definitely be serialized with something like JSON, XML, or Protocol Buffers.
## Dependencies

* sqlite3 
https://www.sqlite.org/index.html
* libconfig
https://hyperrealm.github.io/libconfig/
