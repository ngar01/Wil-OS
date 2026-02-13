# Wil-OS - Root CMakeLists.txt
# ESP-IDF Project Configuration
# Compatible with ESP-IDF v5.0+

# Minimum CMake version required by ESP-IDF
cmake_minimum_required(VERSION 3.16)

# Project-wide settings BEFORE project() call
set(EXTRA_COMPONENT_DIRS 
    "${CMAKE_CURRENT_LIST_DIR}/components/bsp"
    "${CMAKE_CURRENT_LIST_DIR}/components/hal/display"
    "${CMAKE_CURRENT_LIST_DIR}/components/hal/storage"
    "${CMAKE_CURRENT_LIST_DIR}/components/hal/input"
    "${CMAKE_CURRENT_LIST_DIR}/components/services/gui"
    "${CMAKE_CURRENT_LIST_DIR}/components/services/filesystem"
    "${CMAKE_CURRENT_LIST_DIR}/components/services/input_manager"
    "${CMAKE_CURRENT_LIST_DIR}/components/framework"
    "${CMAKE_CURRENT_LIST_DIR}/components/apps/launcher"
    "${CMAKE_CURRENT_LIST_DIR}/components/apps/calculator"
    "${CMAKE_CURRENT_LIST_DIR}/components/apps/file_browser"
)

# Include ESP-IDF build system (sets up toolchain, targets, etc.)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Project declaration
# Version follows Semantic Versioning (MAJOR.MINOR.PATCH)
project(wil-os VERSION 1.0.0 LANGUAGES C CXX ASM)

# ============================================================================
# Project-wide compiler flags and definitions
# ============================================================================

# Optimization level (overridable via menuconfig)
# -Os: Optimize for size (default for embedded)
# -O2: Optimize for speed (uncomment for performance testing)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

# Warnings configuration (enforce code quality)
add_compile_options(
    -Wall                   # Enable all standard warnings
    -Wextra                 # Additional warnings
    -Werror=return-type     # Missing return statement = error
    -Wno-unused-parameter   # Allow unused params (common in callbacks)
    -Wno-missing-field-initializers  # Partial struct init is OK
)

# C standard (C11 with GNU extensions)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# C++ standard (C++17 for hal wrappers, disable exceptions/RTTI)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

# ============================================================================
# Project-wide preprocessor definitions
# ============================================================================

add_compile_definitions(
    # Project identification
    WIL_OS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    WIL_OS_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    WIL_OS_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    
    # Build configuration
    $<$<CONFIG:Debug>:WIL_OS_DEBUG=1>
    $<$<CONFIG:Release>:WIL_OS_RELEASE=1>
    
    # Feature flags (can be overridden in sdkconfig)
    # WIL_OS_ENABLE_SECURE_BOOT
    # WIL_OS_ENABLE_OTA
)

# ============================================================================
# Custom build targets
# ============================================================================

# Target: size-report (analyze binary size)
add_custom_target(size-report
    COMMAND ${CMAKE_SIZE} --format=berkeley ${CMAKE_PROJECT_NAME}.elf
    DEPENDS ${CMAKE_PROJECT_NAME}.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating size report..."
)

# Target: memory-map (generate detailed memory map)
add_custom_target(memory-map
    COMMAND ${CMAKE_OBJDUMP} -h ${CMAKE_PROJECT_NAME}.elf > memory_map.txt
    DEPENDS ${CMAKE_PROJECT_NAME}.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating memory map..."
)

# ============================================================================
# Post-build operations
# ============================================================================

# Copy important build artifacts to easy-to-find location
add_custom_command(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
    COMMAND ${CMAKE_COMMAND} -E echo "Firmware size:"
    COMMAND ${CMAKE_SIZE} --format=berkeley ${CMAKE_PROJECT_NAME}.elf
    COMMENT "Post-build summary"
)

# ============================================================================
# Notes on ESP-IDF CMake Integration
# ============================================================================
# 
# How this works:
# 1. include($ENV{IDF_PATH}/tools/cmake/project.cmake)
#    - Loads ESP-IDF build system
#    - Scans EXTRA_COMPONENT_DIRS for components
#    - Generates dependency graph
#
# 2. project(wil-os ...)
#    - Triggers component registration
#    - Each component's CMakeLists.txt is processed
#    - Generates build rules for main + all components
#
# 3. Build output (in build/ directory):
#    - wil-os.elf (with debug symbols)
#    - wil-os.bin (flashable binary)
#    - wil-os.map (linker map)
#    - bootloader/bootloader.bin
#    - partition_table/partition-table.bin
#
# Component Discovery:
#   Components are discovered in this order:
#   1. IDF_PATH/components (ESP-IDF built-in)
#   2. EXTRA_COMPONENT_DIRS (our custom components)
#   3. PROJECT_DIR/components (fallback)
#   4. PROJECT_DIR/main (mandatory)
#
# Dependency Resolution:
#   If component A declares "REQUIRES component_b":
#   - A's include path gets B's INCLUDE_DIRS
#   - A is linked against B's library
#   - CMake ensures B is built before A
#
# Common pitfalls:
#   - Forgetting to add new component dir to EXTRA_COMPONENT_DIRS
#   - Circular dependencies (CMake will error out)
#   - Missing REQUIRES in component's CMakeLists.txt
#
# Debugging build issues:
#   $ idf.py reconfigure    # Force CMake re-scan
#   $ rm -rf build/         # Nuclear option
#   $ idf.py -v build       # Verbose output
#
# ============================================================================
