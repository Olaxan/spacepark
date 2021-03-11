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
	int cost;
};


