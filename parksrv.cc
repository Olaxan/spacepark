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

parking_server::parking_server(sqlite3*& db)
	: _db(db)
{
}

parking_server::~parking_server()
{
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

			}
		}
	}
}

