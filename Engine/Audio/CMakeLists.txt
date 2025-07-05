set(AUDIO_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(AUDIO_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Define directories for public and private headers
# Gather all source files

file(GLOB_RECURSE AUDIO_PRIVATE_SOURCES
    "${AUDIO_PRIVATE_DIR}/*.cpp"
)

file(GLOB_RECURSE AUDIO_PRIVATE_HEADERS
    "${AUDIO_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE AUDIO_PUBLIC_HEADERS
    "${AUDIO_PUBLIC_DIR}/*.h"
)

add_library(MauEngAudio STATIC
    ${AUDIO_PRIVATE_SOURCES}
    ${AUDIO_PRIVATE_HEADERS}
    ${AUDIO_PUBLIC_HEADERS}
)

target_include_directories(MauEngAudio
    PUBLIC
        ${AUDIO_PUBLIC_DIR}
    PRIVATE
        ${AUDIO_PRIVATE_DIR}
)


if(WITH_WWISE)
    if(DEFINED ENV{WWISESDK})
        set(WWISESDK $ENV{WWISESDK} CACHE PATH "Path to Wwise SDK (from environment)")
    endif()

    message(STATUS "WWISESDK path: ${WWISESDK}")
    add_subdirectory(Wwise)
    target_link_libraries(MauEngAudio PRIVATE MauEngWwiseAudio)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${AUDIO_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${AUDIO_PRIVATE_HEADERS} ${AUDIO_PRIVATE_SOURCES})