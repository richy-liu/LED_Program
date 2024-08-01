#ifndef WIFI_H_
#define WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

void WiFi_Task(void *pvParameters);
bool WiFi_Get_Connected(void);

#define WIFI_TIMEOUT_SECONDS            30

#ifdef __cplusplus
}
#endif

#endif
