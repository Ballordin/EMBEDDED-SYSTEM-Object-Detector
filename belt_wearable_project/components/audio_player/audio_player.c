#include "audio_player.h"

esp_err_t audio_system_init() {
    // 1. Initialize SD Card (SDMMC)
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, NULL);

    // 2. Initialize I2S for MAX98357A
    i2s_chan_config_t chan_cfg = I2S_CHAN_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, &tx_chan, NULL);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16, I2S_SLOT_MODE_MONO),
        .gpio_cfg = { .bclk = GPIO_NUM_4, .ws = GPIO_NUM_5, .dout = GPIO_NUM_6 }
    };
    i2s_channel_init_std_tx(tx_chan, &std_cfg);
    i2s_channel_enable(tx_chan);

    // 3. Create Queue and Task
    audio_queue = xQueueCreate(5, sizeof(audio_msg_t));
    xTaskCreate(audio_task, "audio_task", 4096, NULL, 5, NULL);
    
    return ESP_OK;
}

// The Audio Task: Dedicated to streaming from SD to I2S
void audio_task(void *pvParameters) {
    audio_msg_t msg;
    uint8_t *buffer = malloc(4096); // 4KB DMA Buffer
    
    while (1) {
        // Wait for a message from the sensor task
        if (xQueueReceive(audio_queue, &msg, portMAX_DELAY)) {
            FILE *f = fopen(msg.filepath, "rb");
            if (f) {
                // Skip WAV header (first 44 bytes) for simplicity in this example
                fseek(f, 44, SEEK_SET); 
                
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, 4096, f)) > 0) {
                    size_t bytes_written;
                    i2s_channel_write(tx_chan, buffer, bytes_read, &bytes_written, portMAX_DELAY);
                }
                fclose(f);
            }
        }
    }
}

void audio_play_voice_by_zone(int zone) {
    audio_msg_t msg;
    snprintf(msg.filepath, 64, "/sdcard/zone_%d.wav", zone);
    xQueueSend(audio_queue, &msg, 0);
}