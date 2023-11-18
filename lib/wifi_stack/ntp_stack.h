#ifndef LIB_WIFI_STACK_NTP_STACK_H_
#define LIB_WIFI_STACK_NTP_STACK_H_


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ntp_start();
void get_timestamp(char *timestamp);

#ifdef __cplusplus
}
#endif

#endif  // LIB_WIFI_STACK_NTP_STACK_H_
