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
		bool dock_is_free(int id) const;

		int get_days_docked(int id) const;
		int get_seconds_docked(int id) const;
		int get_fee(int id) const;

		int dock_ship(int id, float weight, const char* license);
		int undock_ship(int id);

		int open(int begin, int end);

	private:

		sqlite3*& _db;
		char* _data[buffer_size];
};
