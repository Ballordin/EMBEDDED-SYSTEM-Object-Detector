// Header Guards: Doesn't let the file being read twice if already defined.
#ifndef HAPTICS_H
#define HAPTICS_H

// Standard Libraries: Includes of standard libraries.
#include <stdint.h>

// Project Libraries: Includes of another libraries/files of the local project or of specific use.
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

// Defines: Numerical Constants, Pin Definition and Nomination, and fixed configurations.
#define MOTOR_PWM_PIN       12 
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // 0 to 1023
#define MOTOR_PWM_FREQ          15000             // test frequency

// Structs: Definition of structures
// External Variables: Variables that need to be manipulated by multiple files at the same time

// Functions Prototypes: All the prototypes of the functions that are used on the source C file. 
esp_err_t haptics_init(void); // Initialize the LEDC PWM peripheral for the ERM motor.
static void vibrate_pulse(uint32_t duty, int count); 
void haptics_update(uint16_t distance_mm, uint8_t range_mode); // Translates distance into vibration intensity and pulses.

#endif