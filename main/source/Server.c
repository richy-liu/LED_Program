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

#define PRESET_STR              ("/?preset=PRESET")

static const char *TAG = "Server";

static httpd_handle_t serverHTTPHandle = NULL;

static LED_Colour staticColour;
static LED_Colour* staticColourPointer = &staticColour;
static LED_Pattern staticPattern = {
    .patternType = Pattern_Type_Repeating,
    .period = 100,
    .colours = &staticColourPointer,
    .numberOfColours = 1,
    .direction = 1,
    .cycles = 1
};
static LED_Pattern* staticPatternPointer = &staticPattern;

static esp_err_t home_handler(httpd_req_t *req);
static esp_err_t null_handler(httpd_req_t *req);
static esp_err_t setHSV_get_handler(httpd_req_t *req);
static esp_err_t setBrightness_get_handler(httpd_req_t *req);
static esp_err_t setSpeed_get_handler(httpd_req_t *req);
static esp_err_t setPreset_get_handler(httpd_req_t *req);

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

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
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

static esp_err_t null_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "NULL handler");
    printf("null: %s", req->uri);
    return ESP_OK;
}

// static void Create_Page(char *start, int *length, uint16_t period)
// {
//     *length += sprintf(start + *length, "<!DOCTYPE html><html>\n");
//     *length += sprintf(start + *length, "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\n");
//     *length += sprintf(start + *length, "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.js\"></script>\n");
//     *length += sprintf(start + *length, "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.css\">\n");
//
//     *length += sprintf(start + *length, "<title>LED Controller</title>\n");
//     *length += sprintf(start + *length, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n");
//     *length += sprintf(start + *length, "<h1 class=\"text-muted\">LED Strip Controller</h1>\n");
//
//     // /*  this creates a list with ON / OFF buttons
//     //     // &nbsp is a non-breaking space; moves next character over
//
//     for (int i = 0; i < NUMBER_OF_PRESETS; i++)
//     {
//         // *length += sprintf(start + *length, "<button onclick=\"sendPreset%d()\" style=\"width: 150px;\">%s</button><br><br>\n", i, Pattern_Pattern_Names[i]);
//         *length += sprintf(start + *length, "<p><a href=\"preset?preset=%d\"><button style=\"width: 150px;\">%s</button></a><br></p>", i, Pattern_Pattern_Names[i]);
//     }
//
//     *length += sprintf(start + *length, "<br>\n");
//
//     *length += sprintf(start + *length, "<script>\n");
//
//     // for (int i = 0; i < NUMBER_OF_PRESETS; i++)
//     // {
//     //     *length += sprintf(start + *length, "function sendPreset%d() {\n", i);
//     //     *length += sprintf(start + *length, "var xhttp = new XMLHttpRequest();\n");
//     //     *length += sprintf(start + *length, "xhttp.open(\"GET\", \"preset?preset=%d\", true);\n", i);
//     //     *length += sprintf(start + *length, "xhttp.send();\n");
//     //     *length += sprintf(start + *length, "}\n");
//     // }
//
//     *length += sprintf(start + *length, "$(document).ready(function() {\n");
//     *length += sprintf(start + *length, "$(\"#setColour\").spectrum( {\n");
//     *length += sprintf(start + *length, "move: function() {\n");
//     *length += sprintf(start + *length, "o = $(\"#setColour\").spectrum(\"get\");\n");
//     *length += sprintf(start + *length, "h = parseFloat(o._originalInput.h);\n");
//     *length += sprintf(start + *length, "s = parseFloat(o._originalInput.s);\n");
//     *length += sprintf(start + *length, "v = parseFloat(o._originalInput.v);\n");
//     *length += sprintf(start + *length, "window.colour = \"hsv?h=\" + Math.round(h) + \"&s=\" + Math.round(s) + \"&v=\" + Math.round(v);\n");
//     *length += sprintf(start + *length, "},\n");
//     *length += sprintf(start + *length, "color: \"#f00\",\n");
//     *length += sprintf(start + *length, "showInput: true,\n");
//     *length += sprintf(start + *length, "showButtons: false\n");
//     *length += sprintf(start + *length, "});\n");
//     *length += sprintf(start + *length, "});\n");
//     *length += sprintf(start + *length, "</script>\n");
//
//     *length += sprintf(start + *length, "<input type=\"text\" id=\"setColour\" />\n");
//
//     *length += sprintf(start + *length, "<br><br>\n");
//
//     *length += sprintf(start + *length, "<div class=\"slidecontainer\">\n");
//     *length += sprintf(start + *length, "<p>Speed: <span id=\"speedSpan\"></span></p>\n");
//     *length += sprintf(start + *length, "<input type=\"range\" min=\"0\" max=\"100\" value=\"%d\" class=\"slider\" id=\"speedRange\">\n", Get_Speed_From_Period(period));
//     *length += sprintf(start + *length, "<p>Brightness: <span id=\"brightnessSpan\"></span></p>\n");
//     *length += sprintf(start + *length, "<input type=\"range\" min=\"0\" max=\"100\" value=\"%d\" class=\"slider\" id=\"brightnessRange\">\n", LED_Get_Brightness() * 100 / 255);
//     *length += sprintf(start + *length, "</div>\n");
//
//
//     *length += sprintf(start + *length, "<script>\n");
//     *length += sprintf(start + *length, "var speedSlider = document.getElementById(\"speedRange\");\n");
//     *length += sprintf(start + *length, "var speedValue = document.getElementById(\"speedSpan\");\n");
//     *length += sprintf(start + *length, "speedValue.innerHTML = speedSlider.value;\n");
//     *length += sprintf(start + *length, "speedSlider.oninput = function() {\n");
//     *length += sprintf(start + *length, "speedValue.innerHTML = this.value;\n");
//     *length += sprintf(start + *length, "window.speed = \"speed?speed=\" + parseFloat(this.value);\n");
//     *length += sprintf(start + *length, "}\n");
//
//     *length += sprintf(start + *length, "var brightnessSlider = document.getElementById(\"brightnessRange\");\n");
//     *length += sprintf(start + *length, "var brightnessValue = document.getElementById(\"brightnessSpan\");\n");
//     *length += sprintf(start + *length, "brightnessValue.innerHTML = brightnessSlider.value;\n");
//     *length += sprintf(start + *length, "brightnessSlider.oninput = function() {\n");
//     *length += sprintf(start + *length, "brightnessValue.innerHTML = this.value;\n");
//     *length += sprintf(start + *length, "window.brightness = \"brightness?brightness=\" + parseFloat(this.value);\n");
//     *length += sprintf(start + *length, "}\n");
//
//     *length += sprintf(start + *length, "window.lastColour = window.colour;\n");
//     *length += sprintf(start + *length, "window.lastSpeed = window.speed;\n");
//     *length += sprintf(start + *length, "window.lastBrightness = window.brightness;\n");
//
//     *length += sprintf(start + *length, "var intervalID = window.setInterval(SendMessage, 100);\n");
//
//     *length += sprintf(start + *length, "function SendMessage() {\n");
//     *length += sprintf(start + *length, "if (window.lastColour != window.colour) {\n");
//     *length += sprintf(start + *length, "window.lastColour = window.colour;\n");
//     *length += sprintf(start + *length, "var xhttp = new XMLHttpRequest();\n");
//     *length += sprintf(start + *length, "xhttp.open(\"GET\", window.lastColour, true);\n");
//     *length += sprintf(start + *length, "xhttp.send();\n");
//     *length += sprintf(start + *length, "}\n");
//     *length += sprintf(start + *length, "if (window.lastSpeed != window.speed) {\n");
//     *length += sprintf(start + *length, "window.lastSpeed = window.speed;\n");
//     *length += sprintf(start + *length, "var xhttp = new XMLHttpRequest();\n");
//     *length += sprintf(start + *length, "xhttp.open(\"GET\", window.lastSpeed, true);\n");
//     *length += sprintf(start + *length, "xhttp.send();\n");
//     *length += sprintf(start + *length, "}\n");
//     *length += sprintf(start + *length, "if (window.lastBrightness != window.brightness) {\n");
//     *length += sprintf(start + *length, "window.lastBrightness = window.brightness;\n");
//     *length += sprintf(start + *length, "var xhttp = new XMLHttpRequest();\n");
//     *length += sprintf(start + *length, "xhttp.open(\"GET\", window.lastBrightness, true);\n");
//     *length += sprintf(start + *length, "xhttp.send();\n");
//     *length += sprintf(start + *length, "}\n");
//     *length += sprintf(start + *length, "}\n");
//
//     *length += sprintf(start + *length, "</script>\n");
//
//     *length += sprintf(start + *length, "</html>\n");
//
// // */
// //  This is a nice drop down menu
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=+1>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<form class=\"form-inline\">");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<div class=\"form-group col-md-3\">");
//     // //counterResponse += sprintf(sResponse + counterResponse, "%s", "<label for=\"styles\">Select Animation:</label>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<select class=\"form-control form-control-sm\" name=\"sCmd\" size=\"5\">");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION1OFF\"selected>All Off</option>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION1ON\">Rainbow</option>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION2ON\">Rainbow Glitter</option>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION3ON\">Confetti</option>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION4ON\">Fire2012</option>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION5ON\">Juggle</option>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION6ON\">BPM</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION7ON\">JustRed</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION8ON\">JustGreen</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION9ON\">JustBlue</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION10ON\">JustPurple</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION11ON\">JustOrange</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION12ON\">Fillnoise8</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION13ON\">Noise16_1</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION14ON\">Noise16_2</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION15ON\">Noise16_3</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION16ON\">Lightning</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION17ON\">Blur</option><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "</select>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<br><br>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<button type=\"submit\" class=\"btn btn-primary\">Select</button>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "</form>");
//     // counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=-1>");
// }

static void Create_Page(char *start)
{
    // Doing it with stpcpy should be faster than with sprintfs...
    char* pointer = start;
    char buffer[100];

    pointer = stpcpy(pointer, "<!DOCTYPE html>\n");
    pointer = stpcpy(pointer, "<html>\n");
    pointer = stpcpy(pointer, "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\n");
    pointer = stpcpy(pointer, "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.js\"></script>\n");
    pointer = stpcpy(pointer, "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/lodash.js/4.17.15/lodash.min.js\"></script>\n");
    pointer = stpcpy(pointer, "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/spectrum/1.8.0/spectrum.min.css\">\n");

    pointer = stpcpy(pointer, "<head>\n");
    pointer = stpcpy(pointer, "<title>LED Controller</title>\n");
    pointer = stpcpy(pointer, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n");

    pointer = stpcpy(pointer, "<style>\n");
    pointer = stpcpy(pointer, ".presetButtons {\n");
    pointer = stpcpy(pointer, "width: 150px;\n");
    // pointer = stpcpy(pointer, "padding: 20px;\n");
    pointer = stpcpy(pointer, "}\n");
    pointer = stpcpy(pointer, "</style>\n");
    pointer = stpcpy(pointer, "</head>\n");

    pointer = stpcpy(pointer, "<body>\n");
    pointer = stpcpy(pointer, "<h1 class=\"text-muted\">LED Strip Controller</h1>\n");

    pointer = stpcpy(pointer, "<div>\n");
    for (int i = 0; i < NUMBER_OF_PRESETS; i++)
    {
        sprintf(buffer, "<button class=\"presetButtons\" onclick=\"sendPreset(%d)\">%s</button><br><br>\n", i, Pattern_Pattern_Names[i]);
        pointer = stpcpy(pointer, buffer);
    }
    pointer = stpcpy(pointer, "</div>\n");
    pointer = stpcpy(pointer, "<br>\n");

    pointer = stpcpy(pointer, "<input type=\"text\" id=\"setColour\" />\n");

    pointer = stpcpy(pointer, "<br><br>\n");

    pointer = stpcpy(pointer, "<div class=\"slidecontainer\">\n");
    pointer = stpcpy(pointer, "<p>Speed: <span id=\"speedSpan\"></span></p>\n");

    sprintf(buffer, "<input type=\"range\" min=\"0\" max=\"100\" value=\"%d\" class=\"slider\" id=\"speedRange\">\n", Get_Speed_From_Period(LED_Get_Period()));
    pointer = stpcpy(pointer, buffer);
    pointer = stpcpy(pointer, "<p>Brightness: <span id=\"brightnessSpan\"></span></p>\n");

    sprintf(buffer, "<input type=\"range\" min=\"0\" max=\"100\" value=\"%d\" class=\"slider\" id=\"brightnessRange\">\n", LED_Get_Brightness() * 100 / 255);
    pointer = stpcpy(pointer, buffer);
    pointer = stpcpy(pointer, "</div>\n");


    pointer = stpcpy(pointer, "<script>\n");

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

    pointer = stpcpy(pointer, "function sendPreset(presetNumber) {\n");
    pointer = stpcpy(pointer, "var xhttp = new XMLHttpRequest();\n");
    pointer = stpcpy(pointer, "xhttp.onreadystatechange = function() {\n");
    pointer = stpcpy(pointer, "if (this.readyState == 4 && this.status == 200) {\n");
    pointer = stpcpy(pointer, "var result = this.responseText.split(\"_\");\n");
    pointer = stpcpy(pointer, "document.getElementById(\"speedRange\").value = result[0];\n");
    pointer = stpcpy(pointer, "document.getElementById(\"speedSpan\").innerHTML = result[0];\n");
    pointer = stpcpy(pointer, "document.getElementById(\"brightnessRange\").value = result[1];\n");
    pointer = stpcpy(pointer, "document.getElementById(\"brightnessSpan\").innerHTML = result[1];\n");
    pointer = stpcpy(pointer, "}\n");
    pointer = stpcpy(pointer, "}\n");
    pointer = stpcpy(pointer, "xhttp.open(\"GET\", \"preset?preset=\" + presetNumber, true);\n");
    pointer = stpcpy(pointer, "xhttp.send();\n");
    // pointer = stpcpy(pointer, "console.log(\"preset?preset=\" + presetNumber);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "var speedSlider = document.getElementById(\"speedRange\");\n");
    pointer = stpcpy(pointer, "var speedValue = document.getElementById(\"speedSpan\");\n");
    pointer = stpcpy(pointer, "speedValue.innerHTML = speedSlider.value;\n");
    pointer = stpcpy(pointer, "speedSlider.oninput = function() {\n");
    pointer = stpcpy(pointer, "speedValue.innerHTML = this.value;\n");
    pointer = stpcpy(pointer, "sendMessageThrottled(\"speed?speed=\" + parseFloat(this.value))\n");
    // pointer = stpcpy(pointer, "console.log(\"speed?speed=\" + parseFloat(this.value));\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "document.getElementById(\"speedSpan\").innerHTML = document.getElementById(\"speedRange\").value;\n");
    pointer = stpcpy(pointer, "document.getElementById(\"speedRange\").oninput = function() {\n");
    pointer = stpcpy(pointer, "document.getElementById(\"speedSpan\").innerHTML = this.value;\n");
    pointer = stpcpy(pointer, "sendMessageThrottled(\"brightness?brightness=\" + parseFloat(this.value))\n");
    // pointer = stpcpy(pointer, "console.log(\"speed?speed=\" + parseFloat(this.value));\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "document.getElementById(\"brightnessSpan\").innerHTML = document.getElementById(\"brightnessRange\").value;\n");
    pointer = stpcpy(pointer, "document.getElementById(\"brightnessRange\").oninput = function() {\n");
    pointer = stpcpy(pointer, "document.getElementById(\"brightnessSpan\").innerHTML = this.value;\n");
    pointer = stpcpy(pointer, "sendMessageThrottled(\"brightness?brightness=\" + parseFloat(this.value))\n");
    // pointer = stpcpy(pointer, "console.log(\"brightness?brightness=\" + parseFloat(this.value));\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "var sendMessageThrottled = _.throttle(sendMessage, 50, { \"trailing\": true });\n");
    pointer = stpcpy(pointer, "function sendMessage(message) {\n");
    pointer = stpcpy(pointer, "var xhttp = new XMLHttpRequest();\n");
    pointer = stpcpy(pointer, "xhttp.open(\"GET\", message, true);\n");
    pointer = stpcpy(pointer, "xhttp.send();\n");
    // pointer = stpcpy(pointer, "console.log(message);\n");
    pointer = stpcpy(pointer, "}\n");

    pointer = stpcpy(pointer, "</script>\n");
    pointer = stpcpy(pointer, "</body>\n");
    pointer = stpcpy(pointer, "</html>\n");
}

/* An HTTP GET handler */
static esp_err_t home_handler(httpd_req_t *req)
{
    char start[8000];

    Create_Page(start);

    printf("%s\n\n", start);
    printf("length: %d\n", strlen(start));
    httpd_resp_send(req, start, strlen(start));

    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setPreset_get_handler(httpd_req_t *req)
{
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
                sprintf(start, "%d_%d", Get_Speed_From_Period(Pattern_Presets[value]->period), LED_Get_Brightness() * 100 / 255);

                httpd_resp_send(req, start, strlen(start));
            }
            else
            {
                httpd_resp_send(req, "", 0);
            }
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setHSV_get_handler(httpd_req_t *req)
{
    char* buf;

    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[4];
            char* c;
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "h", param, sizeof(param)) == ESP_OK) {
                staticColour.hue = strtol(param, &c, 10) * 256 / 100;
            }
            if (httpd_query_key_value(buf, "s", param, sizeof(param)) == ESP_OK) {
                staticColour.saturation = strtol(param, &c, 10) * 255 / 100;
            }
            if (httpd_query_key_value(buf, "v", param, sizeof(param)) == ESP_OK) {
                staticColour.value = strtol(param, &c, 10) * 255 / 100;
            }
            if (LEDConfig_Queue) xQueueOverwriteFromISR(LEDConfig_Queue, &staticPatternPointer, NULL);
            // xQueueSendToBackFromISR(LEDConfig_Queue, &staticPatternPointer, NULL);
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t setBrightness_get_handler(httpd_req_t *req)
{
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
static esp_err_t setSpeed_get_handler(httpd_req_t *req)
{
    char*  buf;

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
                // if (value) value = MAXIMUM_PERIOD - (value - 1) * SPEED_DIVISON_INTERVAL;
                if (LEDPeriod_Queue) xQueueOverwriteFromISR(LEDPeriod_Queue, &value, NULL);
            }
            httpd_resp_send(req, "", 0);
        }
        free(buf);
    }
    return ESP_OK;
}

static TaskHandle_t Get_HTTP_TaskHandle(httpd_handle_t httpHandle)
{
    // httpd_handle_t is a pointer to httpd_data.
    //
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

TaskHandle_t Server_Get_HTTP_Task_Handle(void)
{
    if (serverHTTPHandle)
    {
        return Get_HTTP_TaskHandle(serverHTTPHandle);
    }
    return NULL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 20480;
    config.task_priority = 4;
    config.server_port = 55000;
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
        httpd_register_uri_handler(server, &home);

        ESP_LOGI(TAG, "Server started!");

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");

    return NULL;
}

/* Task to run the http server */
void Server_Task(void *pvParameters)
{
    while (1)
    {
        if (WiFi_Get_Connected())
        {
            serverHTTPHandle = start_webserver();
            vTaskDelay(20000 / portTICK_PERIOD_MS);

            break;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (1) vTaskDelay(portMAX_DELAY);
}
