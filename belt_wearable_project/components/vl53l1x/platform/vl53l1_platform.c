#include "vl53l1_platform.h"

int8_t VL53L1_WriteMulti(VL53_DEV dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    // We create a temporary buffer to hold [Addr MSB, Addr LSB, Data0, Data1...]
    // Given your N16R8 has massive RAM, a 256-byte stack buffer is safe here.
    uint8_t out_buf[count + 2];
    out_buf[0] = (uint8_t)(index >> 8);
    out_buf[1] = (uint8_t)(index & 0xFF);
    memcpy(&out_buf[2], pdata, count);
    
    esp_err_t err = i2c_master_transmit(dev->i2c_handle, out_buf, count + 2, -1);
    return (err == ESP_OK) ? 0 : -1;
}

int8_t VL53L1_ReadMulti(VL53_DEV dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    uint8_t reg_addr[2] = {(uint8_t)(index >> 8), (uint8_t)(index & 0xFF)};
    // Perform the Repeated Start: Write address, then Read N bytes
    esp_err_t err = i2c_master_transmit_receive(dev->i2c_handle, reg_addr, 2, pdata, count, -1);
    return (err == ESP_OK) ? 0 : -1;
}

int8_t VL53L1_WrByte(VL53_DEV dev, uint16_t index, uint8_t data) {
    return VL53L1_WriteMulti(dev, index, &data, 1);
}

int8_t VL53L1_RdByte(VL53_DEV dev, uint16_t index, uint8_t *pdata) {
    return VL53L1_ReadMulti(dev, index, pdata, 1);
}

int8_t VL53L1_WaitMs(VL53_DEV dev, int32_t wait_ms) {
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    return 0;
}