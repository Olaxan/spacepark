#pragma once

// External
#include <memory>
#include <sqlite3.h>

constexpr int buffer_size = 1024;
constexpr int max_clients = 64;

class parking_server
{
	public:

		parking_server(sqlite3*& db);
		~parking_server();

		int open(int begin, int end);
		void close();

	private:

		sqlite3*& _db;
		char* _data[buffer_size];
};
