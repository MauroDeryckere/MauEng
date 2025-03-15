cmake_minimum_required(VERSION 3.27)

# Create the executable target
add_executable(${EXECUTABLE_NAME}
    src/private/main.cpp
)

target_include_directories(${EXECUTABLE_NAME}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
)

find_package(Vulkan REQUIRED)

target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Libs           # Aggregated target from Libs folder (GLFW, GLM, etc.)
        Vulkan::Vulkan # Vulkan SDK (ensure it is installed and found)
)

target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_20)

# Replace this with the correct relative path to vk_layer_settings.txt
set(LAYER_SETTINGS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/Config/vk_layer_settings.txt")
set(LAYER_SETTINGS_DEST "${CMAKE_BINARY_DIR}/$<CONFIG>")

# Copy it after building the target
add_custom_command(
    TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LAYER_SETTINGS_SRC}"
        "${LAYER_SETTINGS_DEST}/vk_layer_settings.txt"
)