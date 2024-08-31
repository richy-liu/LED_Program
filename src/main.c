/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "LED.h"
#include "Colour_Defines.h"
#include "Pattern_Defines.h"
#include "Server.h"
#include "Speed_Period_Converter.h"
#include "WiFi.h"

TaskHandle_t LEDTaskHandle = NULL;
TaskHandle_t WiFiTaskHandle = NULL;
TaskHandle_t ServerTaskHandle = NULL;
TaskHandle_t WaterMarkTaskHandle = NULL;

void WaterMark_Task(void *pvParameters) {
    UBaseType_t highWaterMark;

    // vTaskDelay(portMAX_DELAY);

    while (1) {
        if (WiFiTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(WiFiTaskHandle);
            printf("WiFi High Water Mark: %d\n", highWaterMark);
        }
        if (ServerTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(ServerTaskHandle);
            printf("Server High Water Mark: %d\n", highWaterMark);
        }
        if (Server_Get_HTTP_Task_Handle()) {
            highWaterMark = uxTaskGetStackHighWaterMark(Server_Get_HTTP_Task_Handle());
            printf("HTTP High Water Mark: %d\n", highWaterMark);
        }
        if (LEDTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(LEDTaskHandle);
            printf("LED High Water Mark: %d\n", highWaterMark);
        }
        if (WaterMarkTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(WaterMarkTaskHandle);
            printf("Self High Water Mark: %d\n", highWaterMark);
        }

        printf("\n");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

// #include "esp_chip_info.h"

//     esp_chip_info_t chip_info;
//     esp_chip_info(&chip_info);
//     printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
//             CONFIG_IDF_TARGET,
//             chip_info.cores,
//             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
//             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    
//     printf("silicon revision %d, ", chip_info.revision);
    
//     // printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
//     //         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    
//     fflush(stdout);

    gpio_set_direction(5, GPIO_MODE_OUTPUT);


// xTaskCreatePinnedToCore
    xTaskCreatePinnedToCore(WiFi_Task, "WiFi_Task", 4096, NULL, tskIDLE_PRIORITY, &WiFiTaskHandle, 0);
    xTaskCreatePinnedToCore(Server_Task, "Server_Task", 4096, NULL, 2, &ServerTaskHandle, 0);
    xTaskCreatePinnedToCore(LED_Task, "LED_Task", 8192, NULL, configMAX_PRIORITIES - 1, &LEDTaskHandle, 1);
    // xTaskCreatePinnedToCore(WaterMark_Task, "WaterMark_Task", 4096, NULL, tskIDLE_PRIORITY, &WaterMarkTaskHandle, 0);

    UBaseType_t highWaterMark;

    while (1) {
        // if (WiFiTaskHandle) {
        //     highWaterMark = uxTaskGetStackHighWaterMark(WiFiTaskHandle);
        //     printf("WiFi High Water Mark: %d\n", highWaterMark);
        // }
        if (ServerTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(ServerTaskHandle);
            printf("Server High Water Mark: %d\n", highWaterMark);
        }
        if (Server_Get_HTTP_Task_Handle()) {
            highWaterMark = uxTaskGetStackHighWaterMark(Server_Get_HTTP_Task_Handle());
            printf("HTTP High Water Mark: %d\n", highWaterMark);
        }
        if (LEDTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(LEDTaskHandle);
            printf("LED High Water Mark: %d\n", highWaterMark);
        }
        if (WaterMarkTaskHandle) {
            highWaterMark = uxTaskGetStackHighWaterMark(WaterMarkTaskHandle);
            printf("Self High Water Mark: %d\n", highWaterMark);
        }

        printf("\n");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    esp_restart();
}
