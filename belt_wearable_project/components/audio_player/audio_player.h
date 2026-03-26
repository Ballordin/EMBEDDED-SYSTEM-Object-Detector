// Header Guards: Doesn't let the file being read twice if already defined.
#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

// Standard Libraries: Includes of standard libraries.

// Project Libraries: Includes of another libraries/files of the local project or of specific use.
#include "esp_err.h"
#include "driver/i2s_std.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// Defines: Numerical Constants, Pin Definition and Nomination, and fixed configurations.
#define I2S_BCLK_IO      (4)   // Bit Clock
#define I2S_WS_IO        (5)   // Word Select (L/R Clock)
#define I2S_DO_IO        (6)   // Data Out
static i2s_chan_handle_t tx_chan;
static QueueHandle_t audio_queue;

// Structs: Definition of structures
typedef struct {
    char filepath[64];
} audio_msg_t;

// External Variables: Variables that need to be manipulated by multiple files at the same time

// Functions Prototypes: All the prototypes of the functions that are used on the source C file. 
esp_err_t audio_system_init(void);
void audio_task(void *pvParameters);
void audio_play_voice_by_zone(int zone);

#endif