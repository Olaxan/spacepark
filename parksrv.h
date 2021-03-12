/*
 * This file is part of SPACEPARK.
 *
 * Developed for the VISMA graduate program code challenge.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

// External
#include <memory>
#include <sqlite3.h>

/// The size of the TCP receive buffer.
constexpr int buffer_size = 1024;

/// The maximum number of clients to accept simultanuously.
constexpr int max_clients = 64;

class parking_server
{
	public:

		/**
		 * Create a new SPACEPARK server instance.
		 * The database reference is expected to be opened alread.
		 *
		 * @param db An open sqlite3 database reference.
		 */
		parking_server(sqlite3*& db);
		~parking_server();

		/**
		 * Get the first suitable landing pad
		 * for a spaceship of the specified weight.
		 *
		 * @param weight The ship weight.
		 * @return A free dock id, or -1 if none was found.
		 */
		int get_free_dock(float weight) const;

		/**
		 * Check whether or not the specified dock is occupied.
		 *
		 * @param id The dock ID to check.
		 * @return True if dock is free, false otherwise.
		 */
		bool dock_is_free(int id) const;

		/**
		 * Get number of seconds since a ship last
		 * docked at the specified pad.
		 * Note that this function will not check
		 * whether or not a ship is actually docked.
		 * The user is expected to do that beforehand.
		 *
		 * @param id The id of the dock to check.
		 * @return The number of seconds since a ship last
		 * docked at the specified pad.
		 */
		int get_seconds_docked(int id) const;

		/**
		 * Get the current parking fee for the ship
		 * docked at the specified pad.
		 * Note that this function will not check
		 * whether or not a ship is actually docked.
		 * The user is expected to do that beforehand.
		 *
		 * @param id The id of the dock to check.
		 * @return The fee, in whole interstellar credits.
		 */
		int get_fee(int id) const;

		/**
		 * Register a ship for docking at the specified pad,
		 * checking whether the pad supports the ship weight,
		 * and whether the ship is already docked elsewhere.
		 *
		 * @param id The id of the pad being docked to.
		 * @param weight The weight of the ship in tonnes.
		 * @param license The license string of the ship being docked.
		 * @return A SQL response code.
		 */
		int dock_ship(int id, float weight, const char* license);

		/**
		 * Register a ship for undocking from the specified pad,
		 * checking whether a ship exists at that pad.
		 *
		 * @param id The id of the pad being undocked from.
		 * @return A SQL response code.
		 */
		int undock_ship(int id);

		/**
		 * Opens the parking server, using the specified port range.
		 *
		 * @param begin Beginning of the port range.
		 * @param end End of the port range.
		 * @return A C exit code.
		 */
		int open(int begin, int end);

	private:

		sqlite3*& _db;
		char* _data[buffer_size];
};
