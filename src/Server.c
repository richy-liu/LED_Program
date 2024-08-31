#include <string.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <esp_http_server.h>
#include <sys/param.h>

#include "Pattern_Defines.h"
#include "Server.h"
#include "Speed_Period_Converter.h"
#include "WiFi.h"
#include "LED.h"

#define PRESET_STR              ("/?preset=PRESET")

static const char *TAG = "Server";

static httpd_handle_t serverHTTPHandle = NULL;

static esp_err_t home_handler(httpd_req_t *req);
static esp_err_t null_handler(httpd_req_t *req);
static esp_err_t setHSV_get_handler(httpd_req_t *req);
static esp_err_t setBrightness_get_handler(httpd_req_t *req);
static esp_err_t setSpeed_get_handler(httpd_req_t *req);
static esp_err_t setPreset_get_handler(httpd_req_t *req);
static esp_err_t setReverse_get_handler(httpd_req_t *req);
static esp_err_t setSynchronise_get_handler(httpd_req_t *req);

static const httpd_uri_t favicon = {
    .uri       = "/favicon.ico",
    .method    = HTTP_GET,
    .handler   = null_handler,
    .user_ctx  = NULL
};

// static const httpd_uri_t home = {
static httpd_uri_t home = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = home_handler,
    .user_ctx  = NULL
    // .user_ctx  = homeContext
};

static httpd_uri_t setPreset = {
    .uri       = "/preset",
    .method    = HTTP_GET,
    .handler   = setPreset_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t setHSV = {
    .uri       = "/hsv",
    .method    = HTTP_GET,
    .handler   = setHSV_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t setBrightness = {
    .uri       = "/brightness",
    .method    = HTTP_GET,
    .handler   = setBrightness_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t setSpeed = {
    .uri       = "/speed",
    .method    = HTTP_GET,
    .handler   = setSpeed_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t setReverse = {
    .uri       = "/forward",
    .method    = HTTP_GET,
    .handler   = setReverse_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t setSynchronise = {
    .uri       = "/synchronise",
    .method    = HTTP_GET,
    .handler   = setSynchronise_get_handler,
    .user_ctx  = NULL
};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");

    return ESP_FAIL;
}

static esp_err_t null_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "NULL handler");
    printf("null: %s", req->uri);
    return ESP_OK;
}

static void Create_Page(char *start) {
    // Doing it with stpcpy should be faster than with sprintfs...
    char* pointer = start;
    char buffer[110];

    pointer = stpcpy(pointer, "<!DOCTYPE html>\n");
    pointer = stpcpy(pointer, "<html lang=\"en\">\n");
    pointer = stpcpy(pointer, "<head>\n");

    pointer = stpcpy(pointer, "<meta charset=\"UTF-8\">\n");
    pointer = stpcpy(pointer, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n");
    pointer = stpcpy(pointer, "<title>LED Controller</title>\n");
    pointer = stpcpy(pointer, "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.css\">\n");
   
    pointer = stpcpy(pointer, "<style>\n");
    pointer = stpcpy(pointer, "body {\n");
    pointer = stpcpy(pointer, "font-family: Arial, sans-serif;\n");
    pointer = stpcpy(pointer, "background-color: #f0f0f0;\n");
    pointer = stpcpy(pointer, "margin: 0;\n");
    pointer = stpcpy(pointer, "padding: 20px;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "h1 {\n");
    pointer = stpcpy(pointer, "color: #333;\n");
    pointer = stpcpy(pointer, "text-align: center;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, ".container {\n");
    pointer = stpcpy(pointer, "max-width: 600px;\n");
    pointer = stpcpy(pointer, "margin: 0 auto;\n");
    pointer = stpcpy(pointer, "background: #fff;\n");
    pointer = stpcpy(pointer, "padding: 20px;\n");
    pointer = stpcpy(pointer, "border-radius: 8px;\n");
    pointer = stpcpy(pointer, "box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, ".presetButtons {\n");
    pointer = stpcpy(pointer, "width: 100\%;\n");
    pointer = stpcpy(pointer, "padding: 10px;\n");
    pointer = stpcpy(pointer, "margin: 5px 0;\n");
    pointer = stpcpy(pointer, "border: none;\n");
    pointer = stpcpy(pointer, "border-radius: 5px;\n");
    pointer = stpcpy(pointer, "background-color: #007BFF;\n");
    pointer = stpcpy(pointer, "color: white;\n");
    pointer = stpcpy(pointer, "font-size: 16px;\n");
    pointer = stpcpy(pointer, "cursor: pointer;\n");
    pointer = stpcpy(pointer, "}\n");
    
    pointer = stpcpy(pointer, ".presetButtons:hover {\n");
    pointer = stpcpy(pointer, "background-color: #0056b3;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, ".slidecontainer {\n");
    pointer = stpcpy(pointer, "margin: 20px 0;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, ".slider {\n");
    pointer = stpcpy(pointer, "width: 100\%;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "input[type=\"text\"] {\n");
    pointer = stpcpy(pointer, "width: 100\%;\n");
    pointer = stpcpy(pointer, "padding: 10px;\n");
    pointer = stpcpy(pointer, "margin: 10px 0;\n");
    pointer = stpcpy(pointer, "border-radius: 5px;\n");
    pointer = stpcpy(pointer, "border: 1px solid #ccc;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "label {\n");
    pointer = stpcpy(pointer, "display: block;\n");
    pointer = stpcpy(pointer, "margin: 10px 0 5px;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, ".center {\n");
    pointer = stpcpy(pointer, "text-align: center;\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "</style>\n");
    pointer = stpcpy(pointer, "</head>\n");

    pointer = stpcpy(pointer, "<body>\n");
    pointer = stpcpy(pointer, "<div class=\"container\">\n");
    pointer = stpcpy(pointer, "<h1 class=\"text-muted\">LED Strip Controller</h1>\n");

    pointer = stpcpy(pointer, "<div class=\"preset-container\">\n");
    for (uint32_t i = 0; i < NUMBER_OF_PRESETS; i++) {
        sprintf(buffer, "<button class=\"presetButtons\" onclick=\"sendPreset(%ld)\">%s</button>\n", i, Pattern_Names[i]);
        pointer = stpcpy(pointer, buffer);
    }
    pointer = stpcpy(pointer, "</div>\n");

    pointer = stpcpy(pointer, "<label for=\"setColour\">Set Colour:</label>\n");
    pointer = stpcpy(pointer, "<input type=\"text\" id=\"setColour\" />\n");

    pointer = stpcpy(pointer, "<div class=\"slidecontainer\">\n");
    pointer = stpcpy(pointer, "<label for=\"speedSlider\">Speed: <span id=\"speedValue\"></span></label>\n");
    sprintf(buffer, "<input type=\"range\" min=\"0\" max=\"100\" value=\"%d\" class=\"slider\" id=\"speedSlider\">\n", Get_Speed_From_Period(LED_Get_Period()));
    pointer = stpcpy(pointer, buffer);
    pointer = stpcpy(pointer, "</div>\n");

    pointer = stpcpy(pointer, "<div class=\"slidecontainer\">\n");
    pointer = stpcpy(pointer, "<label for=\"brightnessSlider\">Brightness: <span id=\"brightnessValue\"></span></label>\n");
    sprintf(buffer, "<input type=\"range\" min=\"0\" max=\"100\" value=\"%d\" class=\"slider\" id=\"brightnessSlider\">\n", LED_Get_Brightness() * 100 / 255);
    pointer = stpcpy(pointer, buffer);
    pointer = stpcpy(pointer, "</div>\n");

    pointer = stpcpy(pointer, "<div class=\"center\">\n");
    pointer = stpcpy(pointer, "<label for=\"forwardCheckbox\">Forward: </label>\n");
    sprintf(buffer, "<input type=\"checkbox\" id=\"forwardCheckbox\" onclick=\"toggleForwardCheckbox()\" %s>\n", (LED_Get_Direction() == Forwards ? "checked" : ""));
    pointer = stpcpy(pointer, buffer);
    pointer = stpcpy(pointer, "</div>\n");

    pointer = stpcpy(pointer, "<div class=\"center\">\n");
    pointer = stpcpy(pointer, "<label for=\"synchroniseCheckbox\">Synchronise: </label>\n");
    sprintf(buffer, "<input type=\"checkbox\" id=\"synchroniseCheckbox\" onclick=\"toggleSynchroniseCheckbox()\" %s>\n", (LED_Get_Synchronise() ? "checked" : ""));
    pointer = stpcpy(pointer, buffer);
    pointer = stpcpy(pointer, "</div>\n");

    pointer = stpcpy(pointer, "</div>\n");

    pointer = stpcpy(pointer, "\n");
    pointer = stpcpy(pointer, "\n");
    pointer = stpcpy(pointer, "\n");
    pointer = stpcpy(pointer, "\n");
    pointer = stpcpy(pointer, "\n");
    pointer = stpcpy(pointer, "\n");
    pointer = stpcpy(pointer, "\n");

    pointer = stpcpy(pointer, "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\n");
    pointer = stpcpy(pointer, "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.js\"></script>\n");
    pointer = stpcpy(pointer, "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/lodash.js/4.17.15/lodash.min.js\"></script>\n");
    // pointer = stpcpy(pointer, "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.css\">\n");
    pointer = stpcpy(pointer, "<script>\n");

    pointer = stpcpy(pointer, "function sendPreset(presetNumber) {\n");
    pointer = stpcpy(pointer, "sendMessage(\"preset?preset=\" + presetNumber);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "$(document).ready(function() {\n");
    pointer = stpcpy(pointer, "$(\"#setColour\").spectrum( {\n");
    pointer = stpcpy(pointer, "color: \"#f00\",\n");
    pointer = stpcpy(pointer, "showInput: true,\n");
    pointer = stpcpy(pointer, "showButtons: false\n");
    pointer = stpcpy(pointer, "});\n");
    pointer = stpcpy(pointer, "$(\"#setColour\").on(\"move.spectrum\", setColourFunction);\n");
    pointer = stpcpy(pointer, "$(\"#setColour\").on(\"change.spectrum\", setColourFunction);\n");
    pointer = stpcpy(pointer, "});\n");

    pointer = stpcpy(pointer, "function setColourFunction() {\n");
    pointer = stpcpy(pointer, "o = $(\"#setColour\").spectrum(\"get\");\n");
    pointer = stpcpy(pointer, "h = parseFloat(o._originalInput.h);\n");
    pointer = stpcpy(pointer, "s = parseFloat(o._originalInput.s);\n");
    pointer = stpcpy(pointer, "v = parseFloat(o._originalInput.v);\n");
    pointer = stpcpy(pointer, "sendMessageThrottled(\"hsv?h=\" + Math.round(h) + \"&s=\" + Math.round(s) + \"&v=\" + Math.round(v));\n");
    // pointer = stpcpy(pointer, "console.log(\"hsv?h=\" + Math.round(h) + \"&s=\" + Math.round(s) + \"&v=\" + Math.round(v));\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "function toggleForwardCheckbox() {\n");
    pointer = stpcpy(pointer, "sendMessage(\"forward?value=\" + document.getElementById(\"forwardCheckbox\").checked);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "function toggleSynchroniseCheckbox() {\n");
    pointer = stpcpy(pointer, "sendMessage(\"synchronise?value=\" + document.getElementById(\"synchroniseCheckbox\").checked);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "var speedValue = document.getElementById(\"speedValue\");\n");
    pointer = stpcpy(pointer, "var speedSlider = document.getElementById(\"speedSlider\");\n");
    pointer = stpcpy(pointer, "var brightnessValue = document.getElementById(\"brightnessValue\");\n");
    pointer = stpcpy(pointer, "var brightnessSlider = document.getElementById(\"brightnessSlider\");\n");

    pointer = stpcpy(pointer, "speedValue.innerHTML = speedSlider.value;\n");
    pointer = stpcpy(pointer, "speedSlider.oninput = function() {\n");
    pointer = stpcpy(pointer, "speedValue.innerHTML = this.value;\n");
    pointer = stpcpy(pointer, "sendMessageThrottled(\"speed?speed=\" + parseFloat(this.value));\n");
    // pointer = stpcpy(pointer, "console.log(\"speed?speed=\" + parseFloat(this.value));\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "brightnessValue.innerHTML = brightnessSlider.value;\n");
    pointer = stpcpy(pointer, "brightnessSlider.oninput = function() {\n");
    pointer = stpcpy(pointer, "brightnessValue.innerHTML = this.value;\n");
    pointer = stpcpy(pointer, "sendMessageThrottled(\"brightness?brightness=\" + parseFloat(this.value));\n");
    // pointer = stpcpy(pointer, "console.log(\"brightness?brightness=\" + parseFloat(this.value));\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "var sendMessageThrottled = _.throttle(sendMessage, 50, { \"trailing\": true });\n");

    pointer = stpcpy(pointer, "function sendMessage(message) {\n");
    pointer = stpcpy(pointer, "var xhttp = new XMLHttpRequest();\n");
    pointer = stpcpy(pointer, "xhttp.onreadystatechange = function() {\n");
    pointer = stpcpy(pointer, "if (this.readyState == 4 && this.status == 200) {\n");
    pointer = stpcpy(pointer, "if (this.responseText) {\n");
    pointer = stpcpy(pointer, "var result = this.responseText.split(\"_\");\n");
    pointer = stpcpy(pointer, "speedValue.innerHTML = result[0];\n");
    pointer = stpcpy(pointer, "speedSlider.value = result[0];\n");
    pointer = stpcpy(pointer, "brightnessValue.innerHTML = result[1];\n");
    pointer = stpcpy(pointer, "brightnessSlider.value = result[1];\n");
    pointer = stpcpy(pointer, "forwardCheckbox.checked = result[2];\n");
    pointer = stpcpy(pointer, "synchroniseCheckbox.checked = result[3];\n");
    pointer = stpcpy(pointer, "}\n");
    pointer = stpcpy(pointer, "}\n");
    pointer = stpcpy(pointer, "}\n");
    pointer = stpcpy(pointer, "xhttp.open(\"GET\", message, true);\n");
    pointer = stpcpy(pointer, "xhttp.send();\n");
    // pointer = stpcpy(pointer, "console.log(message);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "</script>\n");
    pointer = stpcpy(pointer, "</body>\n");
    pointer = stpcpy(pointer, "</html>\n");
}

/* An HTTP GET handler */
static esp_err_t home_handler(httpd_req_t *req) {
    char start[8000];

    Create_Page(start);

    // printf("%s\n\n", start);
    printf("length: %d\n", strlen(start));
    httpd_resp_send(req, start, strlen(start));

    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setPreset_get_handler(httpd_req_t *req) {
    char* buf;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[4];
            char* c;
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "preset", param, sizeof(param)) == ESP_OK) {
                uint8_t value = strtol(param, &c, 10);

                if (value < NUMBER_OF_PRESETS && LEDConfig_Queue) xQueueOverwriteFromISR(LEDConfig_Queue, &(Pattern_Presets[value]), NULL);

                char start[50];
                sprintf(start, "%d_%d_%s_%s", Get_Speed_From_Period(LED_Get_Period()), LED_Get_Brightness() * 100 / 255, ((LED_Get_Direction() == Forwards) ? "false" : "true"), (LED_Get_Synchronise() ? "true" : "false"));
                httpd_resp_send(req, start, strlen(start));
            } else {
                httpd_resp_send(req, "", 0);
            }
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setHSV_get_handler(httpd_req_t *req) {
    char* buf;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[4];
            char* c;
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "h", param, sizeof(param)) == ESP_OK) {
                customPattern.colours[0]->hue = strtol(param, &c, 10) * 256 / 100;
            }
            if (httpd_query_key_value(buf, "s", param, sizeof(param)) == ESP_OK) {
                customPattern.colours[0]->saturation = strtol(param, &c, 10) * 255 / 100;
            }
            if (httpd_query_key_value(buf, "v", param, sizeof(param)) == ESP_OK) {
                customPattern.colours[0]->value = strtol(param, &c, 10) * 255 / 100;
            }
            static LED_Pattern* customPatternPointer = &customPattern;
            if (LEDConfig_Queue) xQueueOverwriteFromISR(LEDConfig_Queue, &customPatternPointer, NULL);
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setBrightness_get_handler(httpd_req_t *req) {
    char*  buf;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[4];
            char* c;
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "brightness", param, sizeof(param)) == ESP_OK) {
                uint8_t value = strtol(param, &c, 10) * 255 / 100;
                if (LEDBrightness_Queue) xQueueOverwriteFromISR(LEDBrightness_Queue, &value, NULL);
            }
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setSpeed_get_handler(httpd_req_t *req) {
    char* buf;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[4];
            char* c;
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "speed", param, sizeof(param)) == ESP_OK) {
                uint16_t value = Get_Period_From_Speed((uint8_t) strtol(param, &c, 10));
                // If it returns a 0, then the pattern doesn't move and period should be 0
                if (LEDPeriod_Queue) xQueueOverwriteFromISR(LEDPeriod_Queue, &value, NULL);
            }
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setReverse_get_handler(httpd_req_t *req) {
    // if (LEDReverse_Semaphore) xSemaphoreGive(LEDReverse_Semaphore);
    static bool reverse;
    char* buf;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[6];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
                reverse = strncmp("true", param, 6); // opposite because forward
                // If it returns a 0, then the pattern doesn't move and period should be 0
                if (LEDReverse_Queue) xQueueOverwriteFromISR(LEDReverse_Queue, &reverse, NULL);
            }
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setSynchronise_get_handler(httpd_req_t *req) {
        // if (LEDReverse_Semaphore) xSemaphoreGive(LEDReverse_Semaphore);
    static bool synchronise;
    char* buf;
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {

        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[6];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
                synchronise = !strncmp("true", param, 6);
                // If it returns a 0, then the pattern doesn't move and period should be 0
                if (LEDSynchronise_Queue) xQueueOverwriteFromISR(LEDSynchronise_Queue, &synchronise, NULL);
            }
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;

    // if (LEDSynchronise_Semaphore) xSemaphoreGive(LEDSynchronise_Semaphore);
    // httpd_resp_send(req, "", 0);
    // return ESP_OK;
}

static TaskHandle_t Get_HTTP_TaskHandle(httpd_handle_t httpHandle) {
    // httpd_handle_t is a pointer to httpd_data but the definition of httpd_data
    // is hidden. This function manually calculates the pointer location.

    // struct httpd_data {
    //     httpd_config_t config;                  /*!< HTTPD server configuration */
    //     int listen_fd;                          /*!< Server listener FD */
    //     int ctrl_fd;                            /*!< Ctrl message receiver FD */
    //     int msg_fd;                             /*!< Ctrl message sender FD */
    //     struct thread_data hd_td;               /*!< Information for the HTTPD thread */
    //     struct sock_db *hd_sd;                  /*!< The socket database */
    //     httpd_uri_t **hd_calls;                 /*!< Registered URI handlers */
    //     struct httpd_req hd_req;                /*!< The current HTTPD request */
    //     struct httpd_req_aux hd_req_aux;        /*!< Additional data about the HTTPD request kept unexposed */
    //
    //     /* Array of registered error handler functions */
    //     httpd_err_handler_func_t *err_handler_fns;
    // };
    //
    // struct thread_data {
    //     othread_t handle;   /*!< Handle to thread/task */
    //     enum {
    //         THREAD_IDLE = 0,
    //         THREAD_RUNNING,
    //         THREAD_STOPPING,
    //         THREAD_STOPPED,
    //     } status;           /*!< State of the thread */
    // };

    return (*((TaskHandle_t *) (httpHandle + sizeof(httpd_config_t) + sizeof(int) + sizeof(int) + sizeof(int))));
}

TaskHandle_t Server_Get_HTTP_Task_Handle(void) {
    if (serverHTTPHandle) {
        return Get_HTTP_TaskHandle(serverHTTPHandle);
    }
    return NULL;
}

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 20480;
    config.task_priority = 5;
    config.server_port = 55000;
    config.core_id = 0; // Otherwise LED flickers!!!
    // config.server_port = 55001;
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &favicon);
        httpd_register_uri_handler(server, &setPreset);
        httpd_register_uri_handler(server, &setHSV);
        httpd_register_uri_handler(server, &setBrightness);
        httpd_register_uri_handler(server, &setSpeed);
        httpd_register_uri_handler(server, &setReverse);
        httpd_register_uri_handler(server, &setSynchronise);
        httpd_register_uri_handler(server, &home);

        ESP_LOGI(TAG, "Server started!");

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");

    return NULL;
}

/* Task to run the http server */
void Server_Task(void *pvParameters) {
    while (1) {
        if (WiFi_Get_Connected()) {
            serverHTTPHandle = start_webserver();
            vTaskDelay(20000 / portTICK_PERIOD_MS);

            break;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (1) vTaskDelay(portMAX_DELAY);
}
