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

static int get_free_dock_callback(void* var, int, char** argv, char**)
{
	return (*reinterpret_cast<int*>(var) = atoi(argv[0])) == 0;
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
				get_free_dock_callback,
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

static int dock_is_free_callback(void*, int, char**, char**)
{
	return EXIT_FAILURE;	
}

bool parking_server::dock_is_free(int id) const
{
	// No range check here! We should do that.

	char* statement;
	char* err;
 
	int c;
	int rc = -1;

	if ((c = asprintf(&statement, "SELECT pad_id FROM ships WHERE pad_id = %d;", id)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, dock_is_free_callback, nullptr, &err)) != SQLITE_OK)
		{
			sqlite3_free(err);
		}
		
		free(statement);
	}


	// If the response code is 0, then the callback hasn't aborted the query,
	// and the range constraints haven't failed -- i.e we're fine!
	return (rc == SQLITE_OK);
}

int parking_server::dock_ship(int id, const char*& license)
{
	char* statement;
	char* err;

	int c, rc;

	if ((c = asprintf(&statement, 
					"INSERT INTO ships (pad_id, license) "
					"VALUES (%d, %s);", id, license)) > 0)
	{
		if ((rc = sqlite3_exec(_db, statement, nullptr, nullptr, &err)) != SQLITE_OK)
		{
			fprintf(stderr, "SQL Error %d in dock_ship - %s\n", rc, err);
			sqlite3_free(err);
		}
	}

	return rc;
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

		const int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);

		if ((activity < 0) && (errno != EINTR))
			fprintf(stderr, "A non-fatal selector error occurred!\n");

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

		for (int i = 0; i < max_clients; i++)
		{
			sd = client_sockets[i];

			if (FD_ISSET(sd, &readfds))
			{
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
					msg_head head {};
					memcpy(&head, _data, valread);

					switch (head.type)
					{
						case msg_type::dock_query:
						{

							dock_query_msg msg {};
							memcpy(&msg, _data, sizeof(dock_query_msg));
							
							size_t bytes = sizeof(dock_query_response_msg);
							int dock = get_free_dock(msg.weight);

							dock_query_response_msg response
							{
								msg_head { bytes, 0, msg_type::dock_query_response }, 
								dock	
							};

							if (send(sd, &response, bytes, 0) == static_cast<ssize_t>(bytes))
							{
								fprintf(stdout, "Query response - dock at %d.\n", dock);
							}

							break;
						}
						case msg_type::dock_request:
						{
							size_t bytes = sizeof(dock_change_request_msg);

							dock_change_request_msg msg {};
							memcpy(&msg, _data, bytes); 
							break;
						}
						case msg_type::undock_request:
						{
							dock_change_request_msg msg {};
							memcpy(&msg, _data, sizeof(dock_change_request_msg));
							break;
						}
						default:
							break;
					}
				}
			}
		}
	}
}

