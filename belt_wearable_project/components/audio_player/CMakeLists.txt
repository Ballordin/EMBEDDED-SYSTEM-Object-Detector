idf_component_register(
    SRCS "audio_player.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_i2s fatfs sdmmc
)