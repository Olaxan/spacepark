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

		int get_free_dock(float weight) const;

		int open(int begin, int end);

	private:

		sqlite3*& _db;
		char* _data[buffer_size];
};
