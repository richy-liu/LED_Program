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
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "LED.h"
#include "WiFi.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    printf("Hello world!\n");
    //
    // /* Print chip information */
    // esp_chip_info_t chip_info;
    // esp_chip_info(&chip_info);
    // printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
    //         CONFIG_IDF_TARGET,
    //         chip_info.cores,
    //         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
    //         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    //
    // printf("silicon revision %d, ", chip_info.revision);
    //
    // printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
    //         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    //
    // fflush(stdout);

    gpio_pad_select_gpio(5);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 0);

    UBaseType_t highWaterMark;
    TaskHandle_t LEDTaskHandle = NULL;
    TaskHandle_t WiFiTaskHandle = NULL;

    xTaskCreate(LED_Task, "LED_Task", 4096, NULL, 1, &LEDTaskHandle);
    xTaskCreate(WiFi_Task, "WiFi_Task", 2048, NULL, tskIDLE_PRIORITY, &WiFiTaskHandle);

    while(1) {
        highWaterMark = uxTaskGetStackHighWaterMark(LEDTaskHandle);
        printf("LED High Water Mark: %d\n", highWaterMark);
        highWaterMark = uxTaskGetStackHighWaterMark(WiFiTaskHandle);
        printf("WiFi High Water Mark: %d\n", highWaterMark);

        // gpio_set_level(5, 0);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        // gpio_set_level(5, 1);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    esp_restart();
}
