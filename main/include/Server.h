#ifndef SERVER_H_
#define SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_http_server.h>

TaskHandle_t Server_Get_HTTP_Task_Handle(void);
void Server_Task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif
