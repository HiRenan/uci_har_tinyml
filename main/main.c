#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"

#include "har_model_int8.h"
#include "har_inference.h"
#include "har_feature_extraction.h"

#define I2C_SDA_GPIO GPIO_NUM_8
#define I2C_SCL_GPIO GPIO_NUM_9
#define LED_RUN_GPIO GPIO_NUM_4
#define I2C_PORT I2C_NUM_0
#define MPU6050_ADDR 0x68

#define MPU_REG_PWR_MGMT_1 0x6B
#define MPU_REG_ACCEL_CONFIG 0x1C
#define MPU_REG_GYRO_CONFIG 0x1B
#define MPU_REG_WHO_AM_I 0x75
#define MPU_REG_ACCEL_XOUT_H 0x3B

static const char *TAG = "UCI_HAR_TINYML";
static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t mpu_dev = NULL;

static esp_err_t mpu_write_reg(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_transmit(mpu_dev, data, sizeof(data), pdMS_TO_TICKS(100));
}

static esp_err_t mpu_read_reg(uint8_t reg, uint8_t *value) {
    return i2c_master_transmit_receive(mpu_dev, &reg, 1, value, 1, pdMS_TO_TICKS(100));
}

static esp_err_t mpu_read_bytes(uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_transmit_receive(mpu_dev, &reg, 1, data, len, pdMS_TO_TICKS(100));
}

static int16_t be_i16(const uint8_t *p) {
    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static esp_err_t i2c_init(void) {
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_io_num = I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &i2c_bus), TAG, "falha ao criar barramento I2C");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &mpu_dev), TAG, "falha ao adicionar MPU6050");
    return ESP_OK;
}

static esp_err_t mpu6050_init(void) {
    uint8_t who = 0;
    ESP_RETURN_ON_ERROR(mpu_read_reg(MPU_REG_WHO_AM_I, &who), TAG, "MPU6050 nao respondeu no I2C");
    ESP_LOGI(TAG, "WHO_AM_I=0x%02X", who);

    // Wokwi usa o endereco 0x68 e o WHO_AM_I esperado do MPU6050 e 0x68.
    if (who != 0x68) {
        ESP_LOGW(TAG, "WHO_AM_I inesperado. Continuando porque alguns clones retornam variacoes.");
    }

    ESP_RETURN_ON_ERROR(mpu_write_reg(MPU_REG_PWR_MGMT_1, 0x00), TAG, "falha ao acordar MPU6050");
    vTaskDelay(pdMS_TO_TICKS(50));

    // Faixas usadas no firmware: acelerometro +-2g e giroscopio +-250 dps.
    ESP_RETURN_ON_ERROR(mpu_write_reg(MPU_REG_ACCEL_CONFIG, 0x00), TAG, "falha ao configurar acelerometro");
    ESP_RETURN_ON_ERROR(mpu_write_reg(MPU_REG_GYRO_CONFIG, 0x00), TAG, "falha ao configurar giroscopio");
    return ESP_OK;
}

static esp_err_t mpu6050_read_accel_g(har_accel_sample_t *sample) {
    uint8_t raw[14];
    esp_err_t err = mpu_read_bytes(MPU_REG_ACCEL_XOUT_H, raw, sizeof(raw));
    if (err != ESP_OK) {
        return err;
    }

    const int16_t ax_raw = be_i16(&raw[0]);
    const int16_t ay_raw = be_i16(&raw[2]);
    const int16_t az_raw = be_i16(&raw[4]);

    // Sensibilidade do MPU6050 em +-2g: 16384 LSB/g.
    sample->ax = (float)ax_raw / 16384.0f;
    sample->ay = (float)ay_raw / 16384.0f;
    sample->az = (float)az_raw / 16384.0f;
    return ESP_OK;
}

static void led_set(bool on) {
    gpio_set_level(LED_RUN_GPIO, on ? 1 : 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "Projeto Final TinyML HAR iniciado");
    ESP_LOGI(TAG, "Dataset: UCI HAR Using Smartphones | modelo: MLP %d-%d-%d com pesos int8", HAR_NUM_FEATURES, HAR_HIDDEN_UNITS, HAR_NUM_CLASSES);
    ESP_LOGI(TAG, "Janela: %d amostras a %d Hz | I2C SDA=GPIO%d SCL=GPIO%d", HAR_WINDOW_SIZE, HAR_SAMPLE_RATE_HZ, I2C_SDA_GPIO, I2C_SCL_GPIO);

    gpio_reset_pin(LED_RUN_GPIO);
    gpio_set_direction(LED_RUN_GPIO, GPIO_MODE_OUTPUT);
    led_set(false);

    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(mpu6050_init());
    ESP_LOGI(TAG, "MPU6050 inicializado. Iniciando janelas de inferencia.");

    har_accel_sample_t window[HAR_WINDOW_SIZE];
    float features[HAR_NUM_FEATURES];
    float logits[HAR_NUM_CLASSES];
    int window_id = 0;

    while (true) {
        for (int i = 0; i < HAR_WINDOW_SIZE; ++i) {
            esp_err_t err = mpu6050_read_accel_g(&window[i]);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Erro lendo MPU6050: %s", esp_err_to_name(err));
                vTaskDelay(pdMS_TO_TICKS(500));
                i--;
                continue;
            }
            vTaskDelay(pdMS_TO_TICKS(1000 / HAR_SAMPLE_RATE_HZ));
        }

        har_extract_features_from_accel_window(window, features);
        int pred = har_predict_from_features(features, logits);
        led_set((window_id % 2) == 0);

        printf("HAR TinyML | janela=%d | ax_mean=%.4f ay_mean=%.4f az_mean=%.4f mag_std=%.4f | logits=[%.3f, %.3f, %.3f, %.3f, %.3f, %.3f] | classe=%s\n",
               window_id,
               features[0], features[10], features[20], features[31],
               logits[0], logits[1], logits[2], logits[3], logits[4], logits[5],
               HAR_CLASS_NAMES[pred]);

        window_id++;
    }
}
