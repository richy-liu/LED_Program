#include <stdio.h>
#include <math.h>
#include "Accelerometer.h"
#include "driver/i2c.h"
#include "esp_console.h"
#include "esp_log.h"

#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

#define ACCELEROMETER_SDA_PIN 18
#define ACCELEROMETER_SCL_PIN 19
#define ACCELEROMETER_FREQUENCY 100000
#define ACCELEROMETER_I2C_PORT I2C_NUM_0

#define ACCELEROMETER_ADDR 0b1101000

static const char *TAG = "i2c-example";

// static esp_err_t Get_Value(uint8_t *data, uint8_t address)
// {
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (MODULE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
//     i2c_master_write_byte(cmd, address, ACK_CHECK_EN);
//     i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//     return ret;
// }
void Accelerometer_Write_Value(uint8_t address, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, address, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, value, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        printf("BIG PROBLEM\n");
    }
}

void Accelerometer_Read_Value(uint8_t address, uint8_t* value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, address, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        printf("BIG PROBLEM\n");
    }

    // vTaskDelay(10 / portTICK_RATE_MS);
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, value, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

void Accelerometer_Get_Accel(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t x_h, x_l, y_h, y_l, z_h, z_l;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 59, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &x_h, ACK_VAL);
    i2c_master_read_byte(cmd, &x_l, ACK_VAL);
    i2c_master_read_byte(cmd, &y_h, ACK_VAL);
    i2c_master_read_byte(cmd, &y_l, ACK_VAL);
    i2c_master_read_byte(cmd, &z_h, ACK_VAL);
    i2c_master_read_byte(cmd, &z_l, NACK_VAL);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    *x = x_l | (x_h << 8);
    *y = y_l | (y_h << 8);
    *z = z_l | (z_h << 8);
}

void Accelerometer_Get_Gyro(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t temp_x, temp_y, temp_z;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 67, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ACCELEROMETER_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &temp_x, ACK_VAL);
    i2c_master_read_byte(cmd, x, ACK_VAL);
    i2c_master_read_byte(cmd, &temp_y, ACK_VAL);
    i2c_master_read_byte(cmd, y, ACK_VAL);
    i2c_master_read_byte(cmd, &temp_z, ACK_VAL);
    i2c_master_read_byte(cmd, z, NACK_VAL);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(ACCELEROMETER_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    *x |= (temp_x << 8);
    *y |= (temp_y << 8);
    *z |= (temp_z << 8);
}

void Accelerometer_Initialise(void)
{
    int i2c_master_port = ACCELEROMETER_I2C_PORT;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = ACCELEROMETER_SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = ACCELEROMETER_SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = ACCELEROMETER_FREQUENCY;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);

    // Check the module
    uint8_t whoAmI = 0, powerRegister = 1;
    Accelerometer_Read_Value(117, &whoAmI);

    // Start the module
    Accelerometer_Write_Value(107, 0);
    Accelerometer_Read_Value(107, &powerRegister);

    // Verify the values are correct
    if (whoAmI == 104 && powerRegister == 0) printf("accelerometer connected\n");

    int16_t x = 0, y = 0, z = 0;
    double last_x_double = 1, last_y_double = 1, last_z_double = 1;
    double x_double = 1, y_double = 1, z_double = 1;

    while (1)
    {
        last_x_double = x_double;
        last_y_double = y_double;
        last_z_double = z_double;

        Accelerometer_Get_Accel(&x, &y, &z);
        x_double = (double) x;
        y_double = (double) y;
        z_double = (double) z;

        double dotProduct = last_x_double * x_double + last_y_double * y_double + last_z_double * z_double;
        double cosTheta = dotProduct / (sqrt(x_double * x_double + y_double * y_double + z_double * z_double))
                            / (sqrt(last_x_double * last_x_double + last_y_double * last_y_double + last_z_double * last_z_double));

        // printf("x: %d, y: %d, z: %d\n", x, y, z);
        printf("change from last: %d\n", 1000000 - ((int32_t) (cosTheta * 1000000)));

        vTaskDelay(200 / portTICK_RATE_MS);
    }
}

// static void i2c_test_task(void *arg)
// {
//     int i = 0;
//     int ret;
//     uint32_t task_idx = (uint32_t)arg;
//     uint8_t *data = (uint8_t *)malloc(DATA_LENGTH);
//     uint8_t *data_wr = (uint8_t *)malloc(DATA_LENGTH);
//     uint8_t *data_rd = (uint8_t *)malloc(DATA_LENGTH);
//     uint8_t sensor_data_h, sensor_data_l;
//     int cnt = 0;
//     while (1) {
//         ESP_LOGI(TAG, "TASK[%d] test cnt: %d", task_idx, cnt++);
//         ret = i2c_master_sensor_test(I2C_MASTER_NUM, &sensor_data_h, &sensor_data_l);
//         xSemaphoreTake(print_mux, portMAX_DELAY);
//         if (ret == ESP_ERR_TIMEOUT) {
//             ESP_LOGE(TAG, "I2C Timeout");
//         } else if (ret == ESP_OK) {
//             printf("*******************\n");
//             printf("TASK[%d]  MASTER READ SENSOR( BH1750 )\n", task_idx);
//             printf("*******************\n");
//             printf("data_h: %02x\n", sensor_data_h);
//             printf("data_l: %02x\n", sensor_data_l);
//             printf("sensor val: %.02f [Lux]\n", (sensor_data_h << 8 | sensor_data_l) / 1.2);
//         } else {
//             ESP_LOGW(TAG, "%s: No ack, sensor not connected...skip...", esp_err_to_name(ret));
//         }
//         xSemaphoreGive(print_mux);
//         vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (task_idx + 1)) / portTICK_RATE_MS);
//         //---------------------------------------------------
//         for (i = 0; i < DATA_LENGTH; i++) {
//             data[i] = i;
//         }
//         xSemaphoreTake(print_mux, portMAX_DELAY);
//         size_t d_size = i2c_slave_write_buffer(I2C_SLAVE_NUM, data, RW_TEST_LENGTH, 1000 / portTICK_RATE_MS);
//         if (d_size == 0) {
//             ESP_LOGW(TAG, "i2c slave tx buffer full");
//             ret = i2c_master_read_slave(I2C_MASTER_NUM, data_rd, DATA_LENGTH);
//         } else {
//             ret = i2c_master_read_slave(I2C_MASTER_NUM, data_rd, RW_TEST_LENGTH);
//         }
//
//         if (ret == ESP_ERR_TIMEOUT) {
//             ESP_LOGE(TAG, "I2C Timeout");
//         } else if (ret == ESP_OK) {
//             printf("*******************\n");
//             printf("TASK[%d]  MASTER READ FROM SLAVE\n", task_idx);
//             printf("*******************\n");
//             printf("====TASK[%d] Slave buffer data ====\n", task_idx);
//             disp_buf(data, d_size);
//             printf("====TASK[%d] Master read ====\n", task_idx);
//             disp_buf(data_rd, d_size);
//         } else {
//             ESP_LOGW(TAG, "TASK[%d] %s: Master read slave error, IO not connected...\n",
//                      task_idx, esp_err_to_name(ret));
//         }
//         xSemaphoreGive(print_mux);
//         vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (task_idx + 1)) / portTICK_RATE_MS);
//         //---------------------------------------------------
//         int size;
//         for (i = 0; i < DATA_LENGTH; i++) {
//             data_wr[i] = i + 10;
//         }
//         xSemaphoreTake(print_mux, portMAX_DELAY);
//         //we need to fill the slave buffer so that master can read later
//         ret = i2c_master_write_slave(I2C_MASTER_NUM, data_wr, RW_TEST_LENGTH);
//         if (ret == ESP_OK) {
//             size = i2c_slave_read_buffer(I2C_SLAVE_NUM, data, RW_TEST_LENGTH, 1000 / portTICK_RATE_MS);
//         }
//         if (ret == ESP_ERR_TIMEOUT) {
//             ESP_LOGE(TAG, "I2C Timeout");
//         } else if (ret == ESP_OK) {
//             printf("*******************\n");
//             printf("TASK[%d]  MASTER WRITE TO SLAVE\n", task_idx);
//             printf("*******************\n");
//             printf("----TASK[%d] Master write ----\n", task_idx);
//             disp_buf(data_wr, RW_TEST_LENGTH);
//             printf("----TASK[%d] Slave read: [%d] bytes ----\n", task_idx, size);
//             disp_buf(data, size);
//         } else {
//             ESP_LOGW(TAG, "TASK[%d] %s: Master write slave error, IO not connected....\n",
//                      task_idx, esp_err_to_name(ret));
//         }
//         xSemaphoreGive(print_mux);
//         vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (task_idx + 1)) / portTICK_RATE_MS);
//     }
//     vSemaphoreDelete(print_mux);
//     vTaskDelete(NULL);
// }
