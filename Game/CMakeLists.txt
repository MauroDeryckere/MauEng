# Game CMake file
set(GAME_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(GAME_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")

# Gather all source files
file(GLOB_RECURSE GAME_PRIVATE_SOURCES
    "${GAME_PRIVATE_DIR}/*.cpp"
)

file(GLOB_RECURSE GAME_PRIVATE_HEADERS
    "${GAME_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE GAME_PUBLIC_HEADERS
    "${GAME_PUBLIC_DIR}/*.h"
)


# Create the Game executable
target_sources(${EXECUTABLE_NAME} PRIVATE
    ${GAME_PRIVATE_SOURCES}
    ${GAME_PRIVATE_HEADERS}
    ${GAME_PUBLIC_HEADERS}
)

# Include directories
target_include_directories(${EXECUTABLE_NAME}
    PUBLIC
        ${GAME_PUBLIC_DIR}         # Public headers for Game
    PRIVATE
        ${GAME_PRIVATE_DIR}        # Private headers for Game
)
target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Engine
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${GAME_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${GAME_PRIVATE_HEADERS} ${GAME_PRIVATE_SOURCES})