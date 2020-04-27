/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "WiFi.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define AP_NAME      "TelstraE4E02D"
#define AP_PASSWORD      "qen4bkur47"
// #define AP_NAME      "Telstra0A7E"
// #define AP_PASSWORD      "1557083374"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "WiFi";

static bool connected = 0;
static bool connectedBit = 0;

static char ipAddress[20];

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connected = 0;
        ESP_LOGI(TAG,"Connect to WiFi failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));

        sprintf(ipAddress, IPSTR, IP2STR(&event->ip_info.ip));
        connected = 1;
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = AP_NAME,
            .password = AP_PASSWORD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    // ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    // vEventGroupDelete(s_wifi_event_group);
}

void WiFi_Task(void *pvParameters) {
    wifi_init_sta();

    while (1)
    {
        if (!connected)
        {
            esp_wifi_connect();
            for (int i = 0; i < WIFI_TIMEOUT_SECONDS; i++)
            {
                if (connected)
                {
                    i = WIFI_TIMEOUT_SECONDS;
                }
                else
                {
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
            }
            if (!connected)
            {
                printf("not connected\n");
            }
        }
        else
        {
            gpio_set_level(5, 1);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(5, 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

bool WiFi_Get_Connected(void)
{
    return connected;
}
