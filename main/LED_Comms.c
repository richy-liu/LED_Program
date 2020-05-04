#include <stdio.h>
#include "sdkconfig.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

// The extra
static uint8_t txBuffer[NUMBER_OF_LEDS * 3] = {};
static rmt_item32_t rmtArray[NUMBER_OF_LEDS * 8 * 3 + 1] = {};

// static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 16, 1, 32, 0 }}};
// static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 34, 1, 18, 0 }}};
// decrease the required time to send them all
static const DRAM_ATTR rmt_item32_t bitVal0 = {{{ 11, 1, 27, 0 }}};
static const DRAM_ATTR rmt_item32_t bitVal1 = {{{ 29, 1, 13, 0 }}};
static const DRAM_ATTR rmt_item32_t bitLow = {{{ 1, 1, 2000, 0 }}};

void LED_Comms_Refresh_Data(void)
{
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

    for (int i = 0; i < NUMBER_OF_LEDS * 3; i++, destPointer += 8)
    {
        destPointer->val = (txBuffer[i] & 0b10000000) ? bitVal1.val : bitVal0.val;
        (destPointer + 1)->val = (txBuffer[i] & 0b01000000) ? bitVal1.val : bitVal0.val;
        (destPointer + 2)->val = (txBuffer[i] & 0b00100000) ? bitVal1.val : bitVal0.val;
        (destPointer + 3)->val = (txBuffer[i] & 0b00010000) ? bitVal1.val : bitVal0.val;
        (destPointer + 4)->val = (txBuffer[i] & 0000001000) ? bitVal1.val : bitVal0.val;
        (destPointer + 5)->val = (txBuffer[i] & 0b00000100) ? bitVal1.val : bitVal0.val;
        (destPointer + 6)->val = (txBuffer[i] & 0b00000010) ? bitVal1.val : bitVal0.val;
        (destPointer + 7)->val = (txBuffer[i] & 0b00000001) ? bitVal1.val : bitVal0.val;
    }

    // Last bit sent, make sure it's low for more than 500us while
    (destPointer)->val = bitLow.val;
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

    // printf("ticks: %u, %u, %u, %u, %uMHz\n", t0h_ticks, t0l_ticks, t1h_ticks, t1l_ticks, counterClockMHz);

    // rmt_translator_init(LED_COMMS_CHANNEL, (sample_to_rmt_t) LED_Comms_rmt_adapter);
}

uint8_t* LED_Comms_Get_Tx_Buffer(void)
{
    return txBuffer;
}

void LED_Comms_Send(void)
{
    // vTaskSuspendAll();
    // rmt_write_sample(LED_COMMS_CHANNEL, txBuffer, (NUMBER_OF_LEDS * 3), true);
    rmt_write_items(LED_COMMS_CHANNEL, rmtArray, (NUMBER_OF_LEDS * 8 * 3 + 1), true);
    // xTaskResumeAll();
    rmt_wait_tx_done(LED_COMMS_CHANNEL, pdMS_TO_TICKS(1000));
}
