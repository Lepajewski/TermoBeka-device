#include <string.h>
#include <sys/time.h>

#include "global_config.h"


void get_timestamp(char *timestamp) {
    // get current timestamp in format YYYY-MM-DDTHH:MM:SS.xxxxxx
    char micro_sec_str[7];
    struct timeval tv;

    gettimeofday(&tv, NULL);
    struct tm *seconds = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(char) * 21, "%Y-%m-%dT%H:%M:%S.", seconds);
    snprintf(micro_sec_str, sizeof(char) * 7, "%lu", tv.tv_usec);
    strcat(timestamp, micro_sec_str);
}
