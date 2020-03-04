#include <stdio.h>
#include "sdkconfig.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

#include "LED_Comms.h"
#include "LED.h"

// #define T0H_NS (350)
// #define T0L_NS (1000)
// #define T1H_NS (1000)
// #define T1L_NS (350)
// #define RESET_US (280)

#define T0H_NS (400)
#define T0L_NS (800)
#define T1H_NS (850)
#define T1L_NS (450)
#define RESET_NS (50000)

static uint32_t t0h_ticks = 0;
static uint32_t t1h_ticks = 0;
static uint32_t t0l_ticks = 0;
static uint32_t t1l_ticks = 0;

static void IRAM_ATTR LED_Comms_rmt_adapter(const void *src, rmt_item32_t *dest,
        size_t src_size, size_t wanted_num, size_t *translated_size,
        size_t *item_num)
{
    // if (src == NULL || dest == NULL) {
    if (!(src && dest)) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }

    // typedef struct {
    //     union {
    //         struct {
    //             uint32_t duration0 :15;
    //             uint32_t level0 :1;
    //             uint32_t duration1 :15;
    //             uint32_t level1 :1;
    //         };
    //         uint32_t val;
    //     };
    // } rmt_item32_t;
    //
    // Note that txx_ticks must be < 32768
    // const rmt_item32_t bitVal0 = {{{ t0h_ticks, 1, t0l_ticks, 0 }}};
    // const rmt_item32_t bitVal1 = {{{ t1h_ticks, 1, t1l_ticks, 0 }}};
    static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 16, 1, 32, 0 }}};
    static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 34, 1, 18, 0 }}};

    static size_t size, num;
    static uint8_t *psrc;
    static rmt_item32_t *destPointer;

    size = 0;
    num = 0;
    psrc = (uint8_t *)src;
    destPointer = dest;

    // Unravel while loop for excecution speed
    while (size < src_size && num < wanted_num) {
        if ((*psrc) & 0b10000000) {
            destPointer->val = bitVal1.val;
        } else {
            destPointer->val = bitVal0.val;
        }
        if ((*psrc) & 0b01000000) {
            (destPointer + 1)->val = bitVal1.val;
        } else {
            (destPointer + 1)->val = bitVal0.val;
        }
        if ((*psrc) & 0b00100000) {
            (destPointer + 2)->val =  bitVal1.val;
        } else {
            (destPointer + 2)->val =  bitVal0.val;
        }
        if ((*psrc) & 0b00010000) {
            (destPointer + 3)->val =  bitVal1.val;
        } else {
            (destPointer + 3)->val =  bitVal0.val;
        }
        if ((*psrc) & 0b00001000) {
            (destPointer + 4)->val =  bitVal1.val;
        } else {
            (destPointer + 4)->val =  bitVal0.val;
        }
        if ((*psrc) & 0b00000100) {
            (destPointer + 5)->val =  bitVal1.val;
        } else {
            (destPointer + 5)->val =  bitVal0.val;
        }
        if ((*psrc) & 0b00000010) {
            (destPointer + 6)->val =  bitVal1.val;
        } else {
            (destPointer + 6)->val =  bitVal0.val;
        }
        if ((*psrc) & 0b00000001) {
            (destPointer + 7)->val =  bitVal1.val;
        } else {
            (destPointer + 7)->val =  bitVal0.val;
        }

        destPointer += 8;
        num += 8;
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    if (src == NULL || dest == NULL) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    const rmt_item32_t bit0 = {{{ t0h_ticks, 1, t0l_ticks, 0 }}}; //Logical 0
    const rmt_item32_t bit1 = {{{ t1h_ticks, 1, t1l_ticks, 0 }}}; //Logical 1
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num) {
        for (int i = 0; i < 8; i++) {
            // MSB first
            if (*psrc & (1 << (7 - i))) {
                pdest->val =  bit1.val;
            } else {
                pdest->val =  bit0.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

void LED_Comms_Init(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_COMMS_TX, LED_COMMS_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(LED_COMMS_CHANNEL, 0, 0));

    // Get the frequency of the rmt clock
    uint32_t counterClockHz;
    rmt_get_counter_clock(LED_COMMS_CHANNEL, &counterClockHz);
    uint32_t counterClockMHz = counterClockHz / 1e6;

    t0h_ticks = counterClockMHz * T0H_NS / 1e3;
    t0l_ticks = counterClockMHz * T0L_NS / 1e3;
    t1h_ticks = counterClockMHz * T1H_NS / 1e3;
    t1l_ticks = counterClockMHz * T1L_NS / 1e3;

    printf("ticks: %u, %u, %u, %u, %uMHz\n", t0h_ticks, t0l_ticks, t1h_ticks, t1l_ticks, counterClockMHz);

    rmt_translator_init(LED_COMMS_CHANNEL, (sample_to_rmt_t) LED_Comms_rmt_adapter);

    // // install ws2812 driver
    // led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    // led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
    // if (!strip) {
    //     ESP_LOGE(TAG, "install WS2812 driver failed");
    // }
    // // Clear LED strip (turn off all LEDs)
    // ESP_ERROR_CHECK(strip->clear(strip, 100));
    // // Show simple rainbow chasing pattern
    // ESP_LOGI(TAG, "LED Rainbow Chase Start");
    // while (true) {
    //     for (int i = 0; i < 3; i++) {
    //         for (int j = i; j < CONFIG_EXAMPLE_STRIP_LED_NUMBER; j += 3) {
    //             // Build RGB values
    //             hue = j * 360 / CONFIG_EXAMPLE_STRIP_LED_NUMBER + start_rgb;
    //             led_strip_hsv2rgb(hue, 100, 100, &red, &green, &blue);
    //             // Write RGB values to strip driver
    //             ESP_ERROR_CHECK(strip->set_pixel(strip, j, red, green, blue));
    //         }
    //         // Flush RGB values to LEDs
    //         ESP_ERROR_CHECK(strip->refresh(strip, 100));
    //         vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
    //         strip->clear(strip, 50);
    //         vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
    //     }
    //     start_rgb += 60;
    // }

}

void LED_Comms_Send(LED_Data** LEDData)
{
    uint8_t dataaaa[6] = {10, 10, 0, 20, 20, 0};
    rmt_write_sample(LED_COMMS_CHANNEL, dataaaa, 6, true);
    rmt_wait_tx_done(LED_COMMS_CHANNEL, pdMS_TO_TICKS(1000));
}

static void LED_UART_Store_LED_Data(LED_Data* LEDData)
{
    // for (int i = 0; i < NUMBER_OF_LEDS; i++)
    // {
    //     txBuffer[i * 3] = LEDData[i].red;
    //     txBuffer[i * 3 + 1] = LEDData[i].green;
    //     txBuffer[i * 3 + 2] = LEDData[i].blue;
    // }
}
