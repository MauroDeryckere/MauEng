# Game CMake file
set(GAME_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(GAME_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")

# Create the Game executable
add_executable(${EXECUTABLE_NAME}
    ${GAME_PRIVATE_DIR}/main.cpp
)

# Include directories
target_include_directories(${EXECUTABLE_NAME}
    PUBLIC
        ${GAME_PUBLIC_DIR}         # Public headers for Game
    PRIVATE
        ${GAME_PRIVATE_DIR}        # Private headers for Game
)


# Link the Engine to the Game executable
target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Engine
)

# POST BUILD

set(LAYER_SETTINGS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Renderer/Config")
set(LAYER_SETTINGS_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# Copy it after building the target
add_custom_command(
    TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LAYER_SETTINGS_SRC}"
        "${LAYER_SETTINGS_DEST}/vk_layer_settings.txt"
)