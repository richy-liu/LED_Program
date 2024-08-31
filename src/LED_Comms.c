#include <stdio.h>
#include "sdkconfig.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "LED_Comms.h"
#include "LED.h"

// #define T0H_NS (250)
// #define T0L_NS (700)
// #define T1H_NS (650)
// #define T1L_NS (300)
// #define RESET_NS (50000)

// These are middle values
// #define T0H_NS (400)
// #define T0L_NS (800)
// #define T1H_NS (850)
// #define T1L_NS (450)
// #define RESET_NS (50000)

#define T0H_NS (400)
#define T0L_NS (850)
#define T1H_NS (800)
#define T1L_NS (450)
#define RESET_NS (50000)

// The extra
static uint8_t txBuffer[NUMBER_OF_LEDS * 3] = {};
static rmt_item32_t rmtArray[NUMBER_OF_LEDS * 8 * 3 + 1] = {};

// For WS
// static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 16, 1, 32, 0 }}};
// static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 34, 1, 18, 0 }}};
// decrease the required time to send them all
// static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 8, 1, 17, 0 }}};
// static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 16, 1, 9, 0 }}};
static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 6, 1, 15, 0 }}};
static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 14, 1, 7, 0 }}};
static const DRAM_ATTR rmt_item32_t bitVals[2] = {bitVal0, bitVal1};

// For WS2811
// static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 20, 1, 80, 0 }}};
// static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 48, 1, 52, 0 }}};

// static const DRAM_ATTR rmt_item32_t bitLow = {{{ 1, 1, 2000, 0 }}};
static const DRAM_ATTR rmt_item32_t bitLow = {{{ 500, 0, 500, 0 }}};

// static uint8_t txBuffer[NUMBER_OF_LEDS * 3] = {};
void LED_Comms_Refresh_Data(const uint8_t buffer[], const size_t length) {
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
    rmt_item32_t *destPointer = rmtArray;

    for (int i = 0; i < length; i++, destPointer += 8) {
        destPointer->val = bitVals[(bool) (buffer[i] & (1 << 7))].val;
        (destPointer + 1)->val = bitVals[(bool) (buffer[i] & (1 << 6))].val;
        (destPointer + 2)->val = bitVals[(bool) (buffer[i] & (1 << 5))].val;
        (destPointer + 3)->val = bitVals[(bool) (buffer[i] & (1 << 4))].val;
        (destPointer + 4)->val = bitVals[(bool) (buffer[i] & (1 << 3))].val;
        (destPointer + 5)->val = bitVals[(bool) (buffer[i] & (1 << 2))].val;
        (destPointer + 6)->val = bitVals[(bool) (buffer[i] & (1 << 1))].val;
        (destPointer + 7)->val = bitVals[(bool) (buffer[i] & (1 << 0))].val;
    }

    // Last bit sent, make sure it's low for more than 50us while
    (destPointer)->val = bitLow.val;
}

void LED_Comms_Init(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_COMMS_TX, LED_COMMS_CHANNEL);
    // set counter clock to 20MHz
    config.clk_div = 4;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(LED_COMMS_CHANNEL, 0, 0));

    // Get the frequency of the rmt clock
    uint32_t counterClockHz;
    rmt_get_counter_clock(LED_COMMS_CHANNEL, &counterClockHz);
    uint32_t counterClockMHz = counterClockHz / 1e6;

    uint32_t t0h_ticks = counterClockMHz * T0H_NS / 1e3;
    uint32_t t0l_ticks = counterClockMHz * T0L_NS / 1e3;
    uint32_t t1h_ticks = counterClockMHz * T1H_NS / 1e3;
    uint32_t t1l_ticks = counterClockMHz * T1L_NS / 1e3;

    // printf("ticks: %u, %u, %u, %u, %uMHz\n", t0h_ticks, t0l_ticks, t1h_ticks, t1l_ticks, counterClockMHz);

    // rmt_translator_init(LED_COMMS_CHANNEL, (sample_to_rmt_t) LED_Comms_rmt_adapter);
}

uint8_t* LED_Comms_Get_Tx_Buffer(void)
{
    return txBuffer;
}

void LED_Comms_Send(const uint8_t buffer[], const size_t length) {
    // static uint32_t c = 0;

    LED_Comms_Refresh_Data(buffer, length);
    
    // vTaskSuspendAll();
    // rmt_write_sample(LED_COMMS_CHANNEL, txBuffer, (NUMBER_OF_LEDS * 3), true);
    rmt_write_items(LED_COMMS_CHANNEL, rmtArray, sizeof(rmtArray), true);
    // xTaskResumeAll();
    rmt_wait_tx_done(LED_COMMS_CHANNEL, pdMS_TO_TICKS(1000));

    // printf("txd %lu\n", c);
    // for (int i = 0; i < length; i++) {
    //     printf("%u, ", buffer[i]);
    // }
    // printf("\n");
    // c++;
}
