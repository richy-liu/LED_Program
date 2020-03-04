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
#include "driver/gpio.h"

#include "LED.h"

void app_main(void)
{
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

    LED_Init();

    LED_Task();

    while(1) {
        /* Blink off (output low) */
	// printf("Turning off the LED\n");
        // gpio_set_level(BLINK_GPIO, 0);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
	// printf("Turning on the LED\n");



        gpio_set_level(5, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(5, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Hello world!\n");

    }

    esp_restart();
}
