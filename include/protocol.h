#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <time.h>

#define SERVER_PORT 49160
#define MAX_CLIENTS 10
#define MAX_USERNAME 32
#define MAX_TIMEZONE 64
#define MAX_MESSAGE 270

typedef enum
{
		MSG_LOGIN = 1,
		MSG_GET_TIME,
		MSG_CREATE_TIMEZONE,
		MSG_LIST_TIMEZONES,
		MSG_PING,
		MSG_ERROR,
} msg_type_t;

typedef struct
{
		uint32_t length;
		uint32_t type;
		uint32_t seq;
		char data[MAX_MESSAGE];
} message_t;

typedef struct
{
		int id;
		char name[MAX_TIMEZONE];
		int offset_hours;
		int offset_minutes;
		char location[MAX_TIMEZONE];
		char owner[MAX_USERNAME];
		time_t created_at;
} timezone_t;

typedef struct
{
		char username[MAX_USERNAME];
} login_req_t;

typedef struct
{
		int code;
		char message[MAX_MESSAGE];
} error_resp_t;

#endif
