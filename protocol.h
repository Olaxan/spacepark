#pragma once

constexpr int max_license_len = 64;

enum class msg_type
{
	dock_query,
	dock_query_response,
	dock_request,
	undock_request,
	dock_response,
	undock_response
};

struct msg_head
{
	size_t length;
	unsigned id;
	msg_type type;
};

struct ship_data_head
{
	size_t name_len;
	size_t license_len;
	size_t manufacturer_len;
};

struct ship_data
{
	ship_data_head head;

	float weight;

	char* name;
	char* license_len;
	char* manufacturer_len;
};

struct dock_query_msg
{
	msg_type head;
	float weight;
};

struct dock_query_response_msg
{
	msg_head head;
	int dock_id;
};

struct dock_change_request_msg
{
	msg_head head;
	int dock_id;
	float weight;
	char license[max_license_len];
};

struct dock_response_msg
{
	msg_head head;
	int response;
};

struct undock_response_msg
{
	msg_head head;
	int response;
	int fee;
};


