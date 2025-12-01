#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include "protocol.h"

void get_current_time_string(char *buffer, size_t buffer_size);
int calculate_local_time(const timezone_t *tz, time_t utc_time, struct tm *result);

int create_timezone(timezone_t *tzs, int *count, const char *name, int offset_h, int offset_m, const char *location, const char *owner);
int list_user_timezones(const timezone_t *tzs, int count, const char *username, char *result, size_t resul_size);

#endif
