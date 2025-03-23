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