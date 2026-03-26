#include "haptics.h"

esp_err_t haptics_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = MOTOR_PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = MOTOR_PWM_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    return ledc_channel_config(&ledc_channel);
}

static void vibrate_pulse(uint32_t duty, int count) {
    for (int i = 0; i < count; i++) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(150)); // Pulse duration

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(100)); // Gap between pulses
    }
}

void haptics_update(uint16_t distance_mm, uint8_t range_mode) {
    uint32_t duty = 0;
    int pulses = 0;

    if (range_mode == 2) { // Long Range
        if (distance_mm < 500)       { duty = 1023; pulses = 3; } // 100%
        else if (distance_mm < 2000) { duty = 768;  pulses = 2; } // 75%
        else if (distance_mm < 4000) { duty = 512;  pulses = 1; } // 50%
    } else { // Short Range
        if (distance_mm < 500)       { duty = 1023; pulses = 3; }
        else if (distance_mm < 1000) { duty = 768;  pulses = 2; }
        else if (distance_mm < 2000) { duty = 512;  pulses = 1; }
    }

    if (pulses > 0) {
        vibrate_pulse(duty, pulses);
    }
}