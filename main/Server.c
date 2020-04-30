/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "WiFi.h"

#define PRESET_STR              ("/?preset=PRESET")

static const char *TAG = "Server";

static httpd_handle_t serverHTTPHandle = NULL;

/* String constants for HTTP and HTML */
// const static char httpHeader[] =
//     "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
//
// const static char htmlPageStart[] =
//     "<!DOCTYPE html>"
//     "<html>\n"
//     "<head>\n"
//     "<style>\n"
//     "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
//     ".button { color: black; padding: 16px 40px;\n"
//     "text-decoration: none; font-size: 40px; margin: 2px; cursor: pointer;}\n"
//     "</style>\n"
//     "</head>\n"
//     "<body>\n";
//
// const static char htmlPageEnd[] =
//     "</body>\n"
//     "</html>\n";

// char homeContext[1000] =
// "<!DOCTYPE html>"
// "<html>"
// "<head>"
//   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
//   "<link rel=\"icon\" href=\"data:,\">"
//   "<style>"
//     "body {"
//       "text-align: center;"
//       "font-family: \"Trebuchet MS\", Arial;"
//       "margin-left:auto;"
//       "margin-right:auto;"
//     "}"
//     ".slider {"
//     "  width: 300px;"
//     "}"
//   "</style>"
//   "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>"
// "</head>"
// "<body>"
//   "<h1>ESP32 with Servo</h1>"
//   "<p>Position: <span id=\"servoPos\"></span></p>"
//   "<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\"/>"
//   "<script>"
//     "var slider = document.getElementById(\"servoSlider\");"
//     "var servoP = document.getElementById(\"servoPos\");"
//     "servoP.innerHTML = slider.value;"
//     "slider.oninput = function() {"
//       "slider.value = this.value;"
//       "servoP.innerHTML = this.value;"
//     "}"
//     "$.ajaxSetup({timeout:1000});"
//     "function servo(pos) {"
//       "$.get(\"/?value=\" + pos + \"&\");"
//       "{Connection: close};"
//     "}"
//   "</script>"
// "</body>"
// "</html>";

    // "<!DOCTYPE html>"
    // "<html>\n"
    // "<head>\n"
    // "<style>\n"
    // "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
    // ".button { color: black; padding: 16px 40px;\n"
    // "text-decoration: none; font-size: 40px; margin: 2px; cursor: pointer;}\n"
    // "</style>\n"
    // "</head>\n"
    // "<body>\n"
    //
    // "<form>\n"
    // "<h1><font size=\"7\">Type the new name for \"eees\" and then hit submit.</font></h1>\n"
    // "<input type=\"text\" name=\"reqqddwqd\">\n"
    // "<p><input type=\"submit\" value=\"submit\"></p>\n"
    // "</form>\n"
    //
    // "</body>\n"
    // "</html>\n";

static esp_err_t home_handler(httpd_req_t *req);
static esp_err_t null_handler(httpd_req_t *req);
static esp_err_t hello_get_handler(httpd_req_t *req);
static esp_err_t ctrl_put_handler(httpd_req_t *req);
/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req);

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

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
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
    return ESP_OK;
}

/* An HTTP GET handler */
static esp_err_t home_handler(httpd_req_t *req)
{
    // const char* resp_str = (const char*) req->user_ctx;

    // printf("%s\n",req->uri);
    if (LEDConfig_Queue)
    {
        if (!strncmp(req->uri, PRESET_STR, strlen(PRESET_STR)))
        {
            char* c;
            uint8_t index = strtol(req->uri + strlen(PRESET_STR), &c, 10);
            if (index < NUMBER_OF_PRESETS)
            {
                // xQueueSendToBackFromISR(LEDConfig_Queue, Pattern_Presets + index, NULL);
                xQueueSendToBackFromISR(LEDConfig_Queue, &(Pattern_Presets[index]), NULL);
            }

            // char* c;
            // switch(strtol(req->uri + strlen(PRESET_STR), &c, 10))
            // {
            //     case 0:
            //         xQueueSendToBackFromISR(LEDConfig_Queue, &Pattern_Red_Green_Blue, NULL);
            //         break;
            //     case 1:
            //         xQueueSendToBackFromISR(LEDConfig_Queue, &Pattern_Rainbow, NULL);
            //         break;
            //     case 2:
            //         xQueueSendToBackFromISR(LEDConfig_Queue, &Pattern_Aqua_Wave, NULL);
            //         break;
            //     case 3:
            //         xQueueSendToBackFromISR(LEDConfig_Queue, &Pattern_Black, NULL);
            //         break;
            //     default: break;
            // }
        }
    }

    // httpd_resp_send(req, resp_str, strlen(resp_str));
    int counterHeader = 0, counterResponse = 0;
    char sHeader[500], sResponse[5000];
    char sendString[5500];

//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<html><head><title>ESP_FastLED_Access_Point</title></head><body>");
// //    counterResponse += sprintf(sResponse + counterResponse, "%s", "<font color=\"#FFFFF0\"><body bgcolor=\"#000000\">");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<font color=\"#FFFFF0\"><body bgcolor=\"#151B54\">");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=-1>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<h1>ESP FastLED DemoReel 100<br>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", " Light Controller</h1>");
//         //  This is a nice drop down menue
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=+1>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<form>");
//     //    counterResponse += sprintf(sResponse + counterResponse, "%s", "Select Animation<br>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>Select:</p>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<select name=\"sCmd\" size=\"7\" >");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION1OFF\">All OFF</option>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION1ON\"selected>Rainbow</option>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION2ON\">Rainbow Glitter</option>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION3ON\">Confetti</option>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION4ON\">Sinelon</option>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION5ON\">Juggle</option>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION6ON\">BPM</option><br>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "</select>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<br><br>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "<input type= submit>");
//         counterResponse += sprintf(sResponse + counterResponse, "%s", "</form>");
//             counterResponse += sprintf(sResponse + counterResponse, "%s", "</p>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<form action=\"?sCmd\" >");    // ?sCmd forced the '?' at the right spot
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR>Brightness &nbsp&nbsp");  // perhaps we can show here the current value
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "40");    // this is just a scale depending on the max value; round for better readability
//     counterResponse += sprintf(sResponse + counterResponse, "%s", " %");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<input style=\"width:200px; height:50px\" type=\"range\" name=\"=FUNCTION_200\" id=\"cmd\" value=\"");   // '=' in front of FUNCTION_200 forced the = at the right spot
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "20");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "\" min=10 max=250 step=10 onchange=\"showValue(points)\" />");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR><BR>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<input type=\"submit\">");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "</form>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=-1>");
//     counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "Powered by FastLED<BR><BR>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=-2>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "<font color=\"#FFDE00\">");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "DemoReel 100 by Mark Kriegsman<BR>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "Webserver by Stefan Thesen<BR>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "<font color=\"#FFFFF0\">");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "Gyro Gearloose &nbsp&nbspFeb 2016<BR>");
// counterResponse += sprintf(sResponse + counterResponse, "%s", "</body></html>");
//
// counterHeader += sprintf(sHeader + counterHeader, "%s", "HTTP/1.1 200 OK\r\n");
// counterHeader += sprintf(sHeader + counterHeader, "%s", "Content-Length: ");
// counterHeader += sprintf(sHeader + counterHeader, "%d", counterResponse);
// counterHeader += sprintf(sHeader + counterHeader, "%s", "\r\n");
// counterHeader += sprintf(sHeader + counterHeader, "%s", "Content-Type: text/html\r\n");
// counterHeader += sprintf(sHeader + counterHeader, "%s", "Connection: close\r\n");
// counterHeader += sprintf(sHeader + counterHeader, "%s", "\r\n");


    counterResponse += sprintf(sResponse + counterResponse, "%s", "<html><head><title>LED Controller</title>");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "<style type=\"text/css\">");
    counterResponse += sprintf(sResponse + counterResponse, "%s", ".form-control {");
    counterResponse += sprintf(sResponse + counterResponse, "%s", " width:55px;");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "}");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "</style>");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "</head><body>");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>");
    //counterResponse += sprintf(sResponse + counterResponse, "%s", "<font color=\"#FFFFF0\"><body bgcolor=\"#151B54\">");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=-1>");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<nav class=\"navbar navbar-default\">");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<div class=\"navbar-header\">");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "<div class=\"container\">");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<div class=\"jumbotron\">");
    counterResponse += sprintf(sResponse + counterResponse, "%s", "<h1 class=\"text-muted\">LED Strip Controller</h1><br>");

// /*  this creates a list with ON / OFF buttons
//     // &nbsp is a non-breaking space; moves next character over
    for (int i = 0; i < NUMBER_OF_PRESETS; i++)
    {
        counterResponse += sprintf(sResponse + counterResponse, "<p><a href=\"?preset=PRESET%d\"><button style=\"width: 150px;\">%s</button></a><br></p>", i, Pattern_Pattern_Names[i]);

    }
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p><a href=\"?preset=PRESET0\"><button style=\"width: 150px;\">Rainbow</button></a><br></p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p><a href=\"?preset=PRESET1\"><button style=\"width: 150px;\">Red-Green-Blue</button></a><br></p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p><a href=\"?preset=PRESET2\"><button style=\"width: 150px;\">Aqua Wave</button></a><br></p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p><a href=\"?preset=PRESET3\"><button style=\"width: 150px;\">Off</button></a><br></p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>Rainbow Glitter<a href=\"?preset=FUNCTION2ON\"><button>--ON--</button></a>&nbsp<a href=\"?preset=FUNCTION2OFF\"><button>--OFF--</button></a><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>Confetti &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"?preset=FUNCTION3ON\"><button>--ON--</button></a>&nbsp<a href=\"?preset=FUNCTION3OFF\"><button>--OFF--</button></a><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>Sinelon &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"?preset=FUNCTION4ON\"><button>--ON--</button></a>&nbsp<a href=\"?preset=FUNCTION4OFF\"><button>--OFF--</button></a><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>Juggle&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"?preset=FUNCTION5ON\"><button>--ON--</button></a>&nbsp<a href=\"?preset=FUNCTION5OFF\"><button>--OFF--</button></a></p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>BPM&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"?preset=FUNCTION6ON\"><button>--ON--</button></a>&nbsp<a href=\"?preset=FUNCTION6OFF\"><button>--OFF--</button></a></p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>Function 7&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"?preset=FUNCTION7ON\"><button>--ON--</button></a>&nbsp<a href=\"?preset=FUNCTION7OFF\"><button>--OFF--</button></a></p><br>");
// */
//  This is a nice drop down menu
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=+1>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<form class=\"form-inline\">");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<div class=\"form-group col-md-3\">");
    // //counterResponse += sprintf(sResponse + counterResponse, "%s", "<label for=\"styles\">Select Animation:</label>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<select class=\"form-control form-control-sm\" name=\"sCmd\" size=\"5\">");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION1OFF\"selected>All Off</option>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION1ON\">Rainbow</option>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION2ON\">Rainbow Glitter</option>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION3ON\">Confetti</option>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION4ON\">Fire2012</option>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION5ON\">Juggle</option>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION6ON\">BPM</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION7ON\">JustRed</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION8ON\">JustGreen</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION9ON\">JustBlue</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION10ON\">JustPurple</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION11ON\">JustOrange</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION12ON\">Fillnoise8</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION13ON\">Noise16_1</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION14ON\">Noise16_2</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION15ON\">Noise16_3</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION16ON\">Lightning</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<option value=\"FUNCTION17ON\">Blur</option><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</select>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<br><br>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<button type=\"submit\" class=\"btn btn-primary\">Select</button>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</form>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<FONT SIZE=-1>");

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Slider          this works, however I got http://192.168.4.1/sCmd?FUNCTION_200=80  and the page was not found
    //                 I needed to take the FUNCTION_200=80 apart and call only FUNCTION_200 and assign
    //                 the value (=80) in "react on parameters" (line 512) to new_BRIGHTNESS

    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</p>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<form action=\"?sCmd\" >");    // ?sCmd forced the '?' at the right spot
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR><p class=\"text-muted\">Brightness&nbsp");  // perhaps we can show here the current value
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "100");   // this is just a scale depending on the max value; round for better readability
    // counterResponse += sprintf(sResponse + counterResponse, "%s", " %</p>");
    // //counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<input style=\"width:200px; height:50px\" type=\"range\" name=\"=FUNCTION_200\" id=\"cmd\" value=\"");   // '=' in front of FUNCTION_200 forced the = at the right spot
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "100");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "\" min=10 max=250 step=10 onchange=\"showValue(points)\" />");
    // //counterResponse += sprintf(sResponse + counterResponse, "%s", "<BR>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "<button type=\"submit\" class=\"btn btn-primary\">Select</button>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</form>");
    // //counterResponse += sprintf(sResponse + counterResponse, "%s", "<p>");
    // // counterResponse += sprintf(sResponse + counterResponse, "%s", "<p class=\"text-muted\">Command: ttt</p><BR>");
    //
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</div>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</div>");
    // counterResponse += sprintf(sResponse + counterResponse, "%s", "</body></html>");

    // counterHeader += sprintf(sHeader + counterHeader, "%s", "HTTP/1.1 200 OK\r\n");
    // counterHeader += sprintf(sHeader + counterHeader, "%s", "Content-Length: ");
    // counterHeader += sprintf(sHeader + counterHeader, "%d", counterResponse);
    // counterHeader += sprintf(sHeader + counterHeader, "%s", "\r\n");
    // counterHeader += sprintf(sHeader + counterHeader, "%s", "Content-Type: text/html\r\n");
    // counterHeader += sprintf(sHeader + counterHeader, "%s", "Connection: close\r\n");
    // counterHeader += sprintf(sHeader + counterHeader, "%s", "\r\n");

    // sprintf(sendString, "%s%s", sHeader, sResponse);
    //
    // printf(sendString);
    // httpd_resp_send(req, sendString, strlen(sendString));

    // printf(sResponse);
    httpd_resp_send(req, sResponse, strlen(sResponse));

    return ESP_OK;
}

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
static esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    // char buf;
    // int ret;
    //
    // if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
    //     if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
    //         httpd_resp_send_408(req);
    //     }
    //     return ESP_FAIL;
    // }
    //
    // if (buf == '0') {
    //     /* URI handlers can be unregistered using the uri string */
    //     ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
    //     httpd_unregister_uri(req->handle, "/hello");
    //     httpd_unregister_uri(req->handle, "/echo");
    //     /* Register the custom error handler */
    //     httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
    // }
    // else {
    //     ESP_LOGI(TAG, "Registering /hello and /echo URIs");
    //     httpd_register_uri_handler(req->handle, &hello);
    //     httpd_register_uri_handler(req->handle, &echo);
    //     /* Unregister custom error handler */
    //     httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
    // }
    //
    // /* Respond with empty body */
    // httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    // char buf[100];
    // int ret, remaining = req->content_len;
    //
    // while (remaining > 0) {
    //     /* Read the data for the request */
    //     if ((ret = httpd_req_recv(req, buf,
    //                     MIN(remaining, sizeof(buf)))) <= 0) {
    //         if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
    //             /* Retry receiving if timeout occurred */
    //             continue;
    //         }
    //         return ESP_FAIL;
    //     }
    //
    //     /* Send back the same data */
    //     httpd_resp_send_chunk(req, buf, ret);
    //     remaining -= ret;
    //
    //     /* Log data received */
    //     ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
    //     ESP_LOGI(TAG, "%.*s", ret, buf);
    //     ESP_LOGI(TAG, "====================================");
    // }
    //
    // // End response
    // httpd_resp_send_chunk(req, NULL, 0);
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
    config.task_priority = 2;
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &favicon);
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &ctrl);
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
