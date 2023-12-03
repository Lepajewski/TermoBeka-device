#include <string.h>
#include <sys/time.h>
#include "esp_timer.h"

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

uint64_t get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t s1 = (int64_t)(tv.tv_sec) * 1000;
    int64_t s2 = (tv.tv_usec / 1000);
    return s1 + s2;
}

uint64_t get_time_since_startup_ms() {
    return esp_timer_get_time() / 1000;
}
