idf_component_register(
    SRCS "haptics.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_ledc
)