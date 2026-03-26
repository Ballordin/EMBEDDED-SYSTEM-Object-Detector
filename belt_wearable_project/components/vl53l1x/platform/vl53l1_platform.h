// Header Guards: Doesn't let the file being read twice if already defined.
#ifndef _VL53L1_PLATFORM_H_
#define _VL53L1_PLATFORM_H_

// Standard Libraries: Includes of standard libraries.

// Project Libraries: Includes of another libraries/files of the local project or of specific use.
#include "VL53L1X_types.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Defines: Numerical Constants, Pin Definition and Nomination, and fixed configurations.

// Structs: Definition of structures
typedef struct {
    i2c_master_dev_handle_t i2c_handle; // The ESP-IDF v5.x handle
    uint16_t i2c_address;               // Usually 0x29
} VL53L1_Dev_t;

// External Variables: Variables that need to be manipulated by multiple files at the same time
typedef VL53L1_Dev_t *VL53_DEV;

// Functions Prototypes: All the prototypes of the functions that are used on the source C file. 
int8_t VL53L1_WriteMulti(VL53_DEV dev, uint16_t index, uint8_t *pdata, uint32_t count);
int8_t VL53L1_ReadMulti(VL53_DEV dev, uint16_t index, uint8_t *pdata, uint32_t count);
int8_t VL53L1_WrByte(VL53_DEV dev, uint16_t index, uint8_t data);
int8_t VL53L1_RdByte(VL53_DEV dev, uint16_t index, uint8_t *pdata);
int8_t VL53L1_WaitMs(VL53_DEV dev, int32_t wait_ms);

#endif