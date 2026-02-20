# Main Component CMakeLists.txt
# This is the mandatory entry point component for ESP-IDF projects

idf_component_register(
    SRCS 
        "main.c"
    
    INCLUDE_DIRS 
        "."
    
    REQUIRES
        # Core ESP-IDF components
        esp_system
        esp_timer
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        esp_pm
        
        # Wil-OS custom components (layers bottom-up)
        bsp
        framework
        
)

# The framework component transitively includes:
#   - services (gui, filesystem, input_manager)
#   - hal (display, storage, input)
#   - bsp
# app_launcher
#         app_calculator
#         app_file_browser
# So we only need to explicitly require top-level components
