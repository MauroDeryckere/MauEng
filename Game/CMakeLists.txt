# Game CMake file
set(GAME_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(GAME_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")

# Create the Game executable
target_sources(${EXECUTABLE_NAME} PRIVATE
    ${GAME_PRIVATE_DIR}/main.cpp
    ${GAME_PRIVATE_DIR}/DebugDrawingDemo.cpp
    ${GAME_PRIVATE_DIR}/ECSTestScene.cpp
)

# Include directories
target_include_directories(${EXECUTABLE_NAME}
    PUBLIC
        ${GAME_PUBLIC_DIR}         # Public headers for Game
    PRIVATE
        ${GAME_PRIVATE_DIR}        # Private headers for Game
)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME "VulkanEngine")

target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Engine
)