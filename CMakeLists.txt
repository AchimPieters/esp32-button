idf_component_register(
    SRCS "toggle.c" "button.c" "port.c"
    REQUIRES wolfssl esp_partition json http-parser mdns spi_flash driver
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PRIVATE -DESP_IDF)
