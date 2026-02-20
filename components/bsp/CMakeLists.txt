# BSP Component CMakeLists.txt
# Board Support Package for Wil-OS

idf_component_register(
    SRCS 
        "src/bsp_init.c"
        "src/bsp_gpio.c"
        "src/bsp_spi.c"
    
    INCLUDE_DIRS 
        "include"
    
    REQUIRES
        # ESP-IDF driver components
        driver          # GPIO, SPI, LEDC drivers
        esp_timer       # High-resolution timer
        esp_system      # System utilities
        spi_flash       # Flash operations (for future use)
        
        # FreeRTOS
        freertos
)

# Add compile definitions
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    BSP_COMPONENT_VERSION="1.0.0"
)

# Enable extra warnings for BSP code
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wextra
    -Wconversion
    -Wshadow
)
