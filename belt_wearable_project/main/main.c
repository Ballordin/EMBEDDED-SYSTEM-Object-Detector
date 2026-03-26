#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"

#include "vl53l1_api.h"
#include "vl53l1_platform.h"
#include "haptics.h"
#include "audio_player.h"

#define SLEEP_TIMEOUT_MS 100

static const char *TAG = "TOF_POWER_SAVE";
static const char *TAG = "BELT_MAIN";

#define I2C_SCL_IO           9      
#define I2C_SDA_IO           8      
#define I2C_PORT             0      
#define XSHUT_GPIO           10     
#define TOF_INTERRUPT_PIN    11

uint16_t last_zone = 0; // 0: None, 1: 0.5m, 2: 2m, 3: 4m

void process_distance(uint16_t dist) {
    uint16_t current_zone = 0;
    if (dist < 500) current_zone = 1;
    else if (dist < 2000) current_zone = 2;
    else if (dist < 4000) current_zone = 3;

    if (current_zone != last_zone && current_zone != 0) {
        // 1. Trigger Vibration (Non-blocking)
        haptics_update(dist, 2); 

        // 2. Trigger Voice Line (Via Queue)
        audio_msg_t msg;
        snprintf(msg.filepath, 64, "/sdcard/zone_%d.wav", current_zone);
        xQueueSend(audio_queue, &msg, 0);
        
        last_zone = current_zone;
    }
}

void app_main(void) {
  
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7, // Hardware filter for belt/clothing static
        .trans_queue_depth = 0,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x29,
        .scl_speed_hz = 400000, // 400kHz Fast Mode
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // Initialize our platform structure
    VL53L1_Dev_t vl_dev = {
        .i2c_handle = dev_handle,
        .i2c_address = 0x29
    };

    // ---------------------------------------------------------
    // 2. Low Power GPIO Setup
    // ---------------------------------------------------------
    // Setup XSHUT for hardware reset control
    gpio_config_t xshut_conf = {
        .pin_bit_mask = (1ULL << XSHUT_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&xshut_conf);

    // Setup Interrupt pin to wake ESP32 from Light Sleep
    gpio_config_t int_conf = {
        .pin_bit_mask = (1ULL << TOF_INTERRUPT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // Sensor is open-drain
        .intr_type = GPIO_INTR_LOW_LEVEL,  // Wake on Low Level
    };
    gpio_config(&int_conf);
    
    // Enable the GPIO wakeup source for Light Sleep
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(TOF_INTERRUPT_PIN, GPIO_INTR_LOW_LEVEL);

    // ---------------------------------------------------------
    // 3. Sensor Hardware Wake-up & Initialization
    // ---------------------------------------------------------
    ESP_LOGI(TAG, "Resetting VL53L1X...");
    gpio_set_level(XSHUT_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(XSHUT_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t sensorState = 0;
    while (sensorState == 0) {
        VL53L1X_BootState(&vl_dev, &sensorState);
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    
    VL53L1X_SensorInit(&vl_dev);
    VL53L1X_SetDistanceMode(&vl_dev, 2);      // Long Range (up to 4m)
    VL53L1X_SetTimingBudgetInMs(&vl_dev, 50); // High accuracy window
    VL53L1X_SetInterMeasurementInMs(&vl_dev, 55); 
    VL53L1X_StartRanging(&vl_dev);
    
    ESP_LOGI(TAG, "Ranging Started. Entering Sleep-Loop.");

    // ---------------------------------------------------------
    // 4. Ultra-Low Power Ranging Loop
    // ---------------------------------------------------------
    uint16_t distance;
    uint8_t dataReady = 0;
    const uint64_t sleep_time_us = SLEEP_TIMEOUT_MS * 1000;
    while (1) {
        // SECURITY: Enable a backup timer wakeup in case the sensor hangs
        esp_sleep_enable_timer_wakeup(SLEEP_TIMEOUT_MS * 1000);
        
        esp_light_sleep_start();

        // Identify why we woke up
        esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
        
        uint8_t dataReady = 0;
        VL53L1_RdByte(&vl_dev, 0x00EE, &dataReady); // Check status register directly

        if (cause == ESP_SLEEP_WAKEUP_GPIO && (dataReady & 0x1)) {
            // Normal Operation
            VL53L1X_GetDistance(&vl_dev, &distance);
            VL53L1X_ClearInterrupt(&vl_dev);
            printf("Distance_mm: %u\n", distance);
        } 
        else if (cause == ESP_SLEEP_WAKEUP_TIMER) {
            // WATCHDOG TRIGGERED: Sensor didn't respond in time
            ESP_LOGE(TAG, "Sensor Timeout! Attempting Hardware Recovery...");
            
            // SECURITY: Hardware Reset the sensor via XSHUT
            gpio_set_level(XSHUT_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(50));
            gpio_set_level(XSHUT_GPIO, 1);
            
            // Re-initialize (The ULD handles the 100-byte config reload)
            VL53L1X_SensorInit(&vl_dev);
            VL53L1X_StartRanging(&vl_dev);
        }
    }
}