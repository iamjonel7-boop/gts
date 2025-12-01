#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "time_service.h"
#include "protocol.h"

void get_current_time_string(char *buffer, size_t buffer_size)
{
		time_t now = time(NULL);
		struct tm *tm = gmtime(&now);

		snprintf(buffer, buffer_size, "UTC: %.4d-%.2d-%.2dT%.2d:%.2d%.2dZ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

int calculate_local_time(const timezone_t *tz, time_t utc_time, struct tm *result)
{
		time_t local_time = utc_time + (tz->offset_hours * 3600) + (tz->offset_minutes * 60);
		struct tm *tm = gmtime(&local_time);

		if(tm)
		{
				*result = *tm;
				return 0;
		}
		return -1;
}

int create_timezone(timezone_t *timezones, int *count, const char *name, int offset_h, int offset_m, const char *location, const char *owner)
{
		if(*count >= MAX_CLIENTS) return -1;

		timezone_t *tz = &timezones[*count];
		tz->id = *count + 1;
		strncpy(tz->name, name, MAX_TIMEZONE -1);
		tz->offset_hours = offset_h;
		tz->offset_minutes = offset_m;
		strncpy(tz->location, location, MAX_TIMEZONE - 1);
		strncpy(tz->owner, owner, MAX_USERNAME - 1);
		tz->created_at = time(NULL);

		(*count)++;
		return tz->id;
}

int list_user_timezones(const timezone_t *timezones, int count, const char *username, char *result, size_t result_size)
{
		char *ptr = result;
		int found = 0;

		ptr += snprintf(ptr, result_size - (ptr - result), "Your timezones:\n");

		for(int i = 0; i < count; i++)
		{
				if(strcmp(timezones[i].owner, username) == 0)
				{
						ptr += snprintf(ptr, result_size - (ptr - result), "ID: %d, Name: %s, Offset: %+03d:%02d, Location: %s\n", timezones[i].id, timezones[i].name, timezones[i].offset_hours, timezones[i].offset_minutes, timezones[i].location);
						found = 1;
				}
		}

		if(!found)
				strncpy(result, "No timezones found. Use 'create' to add one.", result_size);

		return found;
}
