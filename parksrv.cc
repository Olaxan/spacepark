#include "parksrv.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  

#include <functional>
#include <memory>

#include "protocol.h"

parking_server::parking_server(sqlite3*& db)
	: _db(db)
{
}

parking_server::~parking_server()
{
}


/**
 * This function is run once for every returned row from a SQL query,
 * and sets the variable 'var' equal to the first column as an integer.
 *
 * @param var The input variable which will be set.
 * @return This function always returns EXIT_SUCCESS.
 */
static int get_first_as_integer(void* var, int, char** argv, char**)
{
	*reinterpret_cast<int*>(var) = atoi(argv[0]);
	return EXIT_SUCCESS;
}

/**
 * This function is similar to get_first_as_integer(),
 * but returns 1 (i.e EXIT_FAILURE) if the column
 * was evaluated to 0.
 * @param var The input variable which will be set.
 * @return 1 if var is 0, 0 otherwise.
 */
static int get_first_as_integer_not_zero(void* var, int, char** argv, char**)
{
	return (*reinterpret_cast<int*>(var) = atoi(argv[0])) == 0;
}

/**
 * This function will always return failure,
 * causing SQLite to report SQLITE_ABORT
 * -- functioning as a sort of EXISTS-equivalent
 */
static int row_exists_callback(void*, int, char**, char**)
{
	return EXIT_FAILURE;	
}

int parking_server::get_free_dock(float weight) const
{
	char* statement;
	char* err;

	int dock = -1;
	int c, rc;

	if ((c = asprintf(&statement,
			"SELECT pad_id FROM pads "
			"WHERE pad_id NOT IN ("
			"SELECT pad_id FROM ships) "
			"AND max_weight > %f "
			"LIMIT 1;", weight)) > 0)
	{

		if ((rc = sqlite3_exec(_db, statement,
				get_first_as_integer_not_zero,
				&dock, 
				&err)) > 0)
		{
			fprintf(stderr, "SQL Error %d in get_free_dock - %s\n", rc, err);
			sqlite3_free(err);
		}
	}

	free(statement);

	return dock;
}

bool parking_server::dock_is_free(int id) const
{
	// No range check here! We should do that.
	// In fact, this whole method is pretty stupid.

	char* statement;
	char* err;
 
	int c;
	int rc = -1;

	if ((c = asprintf(&statement, "SELECT pad_id FROM ships WHERE pad_id = %d;", id)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, row_exists_callback, nullptr, &err)) != SQLITE_OK)
		{
			sqlite3_free(err);
		}
		
		free(statement);
	}

	// If the response code is 0, then the callback hasn't aborted the query,
	// and the range constraints haven't failed -- i.e. the dock is free!
	return (rc == SQLITE_OK);
}

int parking_server::get_seconds_docked(int id) const
{
	char* statement;
	char* err;

	int seconds = -1;

	int c;
	int rc = -1;

	if ((c = asprintf(&statement, 
					"SELECT CAST ("
					"(JulianDay('NOW') - JulianDay(date)) * 24 * 60 * 60"
					" AS INTEGER) "
					"FROM ships "
					"WHERE pad_id = %d;", id)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, 
						get_first_as_integer, &seconds, &err)) != SQLITE_OK)
		{
			fprintf(stderr, "SQL Error %d in get_seconds_docked - %s\nQuery: %s\n", rc, err, statement);
			sqlite3_free(err);
		}

		free(statement);
	}

	return seconds;
}

int parking_server::get_fee(int id) const
{
	char* statement;
	char* err;

	int fee = -1;

	int c;
	int rc = -1;

	if ((c = asprintf(&statement, 
					"WITH span AS ("
					"\n    SELECT"
					"\n    (JulianDay('NOW') - JulianDay(date))"
					"\n    AS days"
					"\n    FROM ships"
					"\n    WHERE pad_id = %d"
					"\n    )"
					"\nSELECT"
					"\nCASE"
					"\n    WHEN days > 1 THEN"
					"\n    ROUND(days + 0.5) * cost_day"
					"\n    ELSE"
					"\n    ROUND(days * 24 + 0.5) * cost_hour"
					"\n    END fee"
					"\nFROM pads, span"
					"\nWHERE pad_id = %d", id, id)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, 
						get_first_as_integer, &fee, &err)) != SQLITE_OK)
		{
			fprintf(stderr, "SQL Error %d in get_fee - %s\nQuery: %s\n", rc, err, statement);
			sqlite3_free(err);
		}

		free(statement);
	}

	return fee;
}

int parking_server::dock_ship(int id, float weight, const char* license)
{
	char* statement;
	char* err;

	int c, rc;

	if ((c = asprintf(&statement, 
					"INSERT INTO ships (pad_id, weight, license, date) "
					"VALUES (%d, %f, '%s', DATETIME('NOW'));", id, weight, license)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, nullptr, nullptr, &err)) != SQLITE_OK)
		{
			fprintf(stderr, "SQL Error %d in dock_ship - %s\nQuery: %s\n", rc, err, statement);
			sqlite3_free(err);
		}

		free(statement);
	}

	return rc;
}

int parking_server::undock_ship(int id)
{
	char* statement;
	char* err;
	
	int c, rc;

	if ((c = asprintf(&statement,
					"DELETE FROM ships WHERE pad_id = %d;", id)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, nullptr, nullptr, &err)) != SQLITE_OK)
		{
			fprintf(stderr, "SQL Error %d in undock_ship - %s\nQuery: %s\n", rc, err, statement);
			sqlite3_free(err);
		}

		free(statement);
	}

	return (sqlite3_changes(_db) > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int parking_server::open(int begin, int end)
{
	int opt = true;
	int master_socket, addrlen, new_socket, valread, sd;
	int client_sockets[max_clients];
	struct sockaddr_in address {};

	fd_set readfds;

	for (int i = 0; i < max_clients; i++)
		client_sockets[i] = 0;

	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		fprintf(stderr, "Failed to create socket.\n");
		return EXIT_FAILURE;
	}

	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, 
				reinterpret_cast<char*>(&opt), sizeof(opt)) < 0)
	{
		fprintf(stderr, "Failed to configure socket.\n");
		return EXIT_FAILURE;
	}

	int port = begin;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	while (bind(master_socket, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0)
	{
		if (port > end)
		{
			fprintf(stderr, "Failed to bind port.\n");
			return EXIT_FAILURE;
		}

		address.sin_port = htons(port++);
	}

	fprintf(stdout, "Listening on %d.\n", port);

	if (listen(master_socket, 3) < 0)
	{
		fprintf(stderr, "Failed to start listening.\n");
		return EXIT_FAILURE;
	}

	addrlen = sizeof(address);

	// Sockets are set up, enter main listening loop.
	while (true)
	{
		// Clear socket set and add master socket.
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
		int max_sd = master_socket;

		// Add client sockets to set.
		for (int i = 0; i < max_clients; i++)
		{
			sd = client_sockets[i];

			if (sd > 0)
				FD_SET(sd, &readfds);

			if (sd > max_sd)
				max_sd = sd;
		}

		// Select a FD ready for I/O
		const int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);

		if ((activity < 0) && (errno != EINTR))
			fprintf(stderr, "A non-fatal selector error occurred!\n");

		// There is activity on the socket
		// -- accept the connection and add it
		// to a free client socked.
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket, 
							reinterpret_cast<struct sockaddr*>(&address),
							reinterpret_cast<socklen_t*>(&addrlen))) < 0)
			{
				fprintf(stderr, "Error when accepting connection.\n");
				return EXIT_FAILURE;
			}

			fprintf(stdout, "New connection (sdf %d) from %s:%d.\n", 
					new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			for (int i = 0; i < max_clients; i++)
			{
				if (client_sockets[i] == 0)
				{
					client_sockets[i] = new_socket;
					break;
				}
			}
		}

		// Loop through all clients and act on those
		// who are connected (i.e FD is set)
		for (int i = 0; i < max_clients; i++)
		{
			sd = client_sockets[i];

			if (FD_ISSET(sd, &readfds))
			{
				// A return value of zero indicates EOS,
				// so we can disconnect the client.
				if ((valread = read(sd, _data, buffer_size)) == 0)
				{
					getpeername(sd, 
							reinterpret_cast<struct sockaddr*>(&address),
							reinterpret_cast<socklen_t*>(&addrlen));

					fprintf(stdout, "Disconnected %s:%d.\n",
							inet_ntoa(address.sin_addr), ntohs(address.sin_port));

					close(sd);
					client_sockets[i] = 0;
				}
				else if (valread > -1)
				{

					// Read the message head and decide what to do
					// with the rest of the message.
					msg_head head {};
					memcpy(&head, _data, valread);

					switch (head.type)
					{
						case msg_type::dock_query:
						{

							// A client wants to know if there are any free
							// landing pads!

							size_t msg_bytes = sizeof(dock_query_msg);
							size_t rsp_bytes = sizeof(dock_query_response_msg);

							dock_query_msg msg {};
							memcpy(&msg, _data, msg_bytes); 

							int dock = get_free_dock(msg.weight);

							dock_query_response_msg rsp 
							{
								msg_head { rsp_bytes, 0, msg_type::dock_query_response }, 
								dock	
							};

							if (send(sd, &rsp, rsp_bytes, 0) == static_cast<ssize_t>(rsp_bytes))
								fprintf(stdout, "Query response (%lu bytes) sent.\n", rsp_bytes);

							break;
						}
						case msg_type::dock_request:
						{

							// A client wants to register a docking ship!

							size_t msg_bytes = sizeof(dock_change_request_msg);
							size_t rsp_bytes = sizeof(dock_response_msg);

							dock_change_request_msg msg {};
							memcpy(&msg, _data, msg_bytes); 

							int rc = dock_ship(msg.dock_id, msg.weight, msg.license); 

							dock_response_msg rsp 
							{
								msg_head { rsp_bytes, 0, msg_type::dock_response },
								rc
							};

							if (send(sd, &rsp, rsp_bytes, 0) == static_cast<ssize_t>(rsp_bytes))
								fprintf(stdout, "Dock request (%lu bytes) sent.\n", rsp_bytes);

							break;
						}
						case msg_type::undock_request:
						{

							// A client wants to register an undocking ship!

							size_t msg_bytes = sizeof(dock_change_request_msg);
							size_t rsp_bytes = sizeof(undock_response_msg);

							dock_change_request_msg msg {};
							memcpy(&msg, _data, msg_bytes); 

							int fee = get_fee(msg.dock_id);
							int rc = undock_ship(msg.dock_id);

							undock_response_msg rsp 
							{
								msg_head { rsp_bytes, 0, msg_type::undock_response },
								rc,
								fee
							};

							if (send(sd, &rsp, rsp_bytes, 0) == static_cast<ssize_t>(rsp_bytes))
								fprintf(stdout, "Undock request (%lu bytes) sent.\n", rsp_bytes);

							break;
						}
						default:
							break;
					}
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

