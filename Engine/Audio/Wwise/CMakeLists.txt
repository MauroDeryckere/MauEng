set(WWISE_AUDIO_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(WWISE_AUDIO_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Define directories for public and private headers
# Gather all source files

file(GLOB_RECURSE WWISE_AUDIO_PRIVATE_SOURCES
    "${WWISE_AUDIO_PRIVATE_DIR}/*.cpp"
)

file(GLOB_RECURSE WWISE_AUDIO_PRIVATE_HEADERS
    "${WWISE_AUDIO_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE WWISE_AUDIO_PUBLIC_HEADERS
    "${WWISE_AUDIO_PUBLIC_DIR}/*.h"
)

add_library(MauEngWwiseAudio STATIC
    ${WWISE_AUDIO_PRIVATE_SOURCES}
    ${WWISE_AUDIO_PRIVATE_HEADERS}
    ${WWISE_AUDIO_PUBLIC_HEADERS})

target_include_directories(MauEngWwiseAudio PRIVATE
    ${WWISESDK}/include
)
target_include_directories(MauEngWwiseAudio
    PUBLIC
        ${WWISE_AUDIO_PUBLIC_DIR}
    PRIVATE
        ${WWISE_AUDIO_PRIVATE_DIR}
        ${AUDIO_PUBLIC_DIR}
)

target_link_directories(MauEngWwiseAudio PRIVATE
    $<$<CONFIG:Debug>:${WWISESDK}/win32_vc170/Debug/lib>
    $<$<CONFIG:Release>:${WWISESDK}/win32_vc170/release/lib>
)
if(NOT WIN32)
    message(WARNING "Wwise Audio is only supported on Windows. Skipping MauEngWwiseAudio build.")
    return()
endif()

file(GLOB WWISE_LIBS_RELEASE86 "${WWISESDK}/Win32_vc170/Release/lib/*.lib")
file(GLOB WWISE_LIBS_DEBUG86 "${WWISESDK}/Win32_vc170/Debug/lib/*.lib")
file(GLOB WWISE_LIBS_RELEASE64 "${WWISESDK}/x64_vc170/Release/lib/*.lib")
file(GLOB WWISE_LIBS_DEBUG64   "${WWISESDK}/x64_vc170/Debug/lib/*.lib")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WWISE_LIBS_RELEASE ${WWISE_LIBS_RELEASE64})
    set(WWISE_LIBS_DEBUG   ${WWISE_LIBS_DEBUG64})
else()
    set(WWISE_LIBS_RELEASE ${WWISE_LIBS_RELEASE86})
    set(WWISE_LIBS_DEBUG   ${WWISE_LIBS_DEBUG86})
endif()

target_link_libraries(MauEngWwiseAudio PRIVATE
    MauEngCore

    $<$<CONFIG:Debug>:${WWISE_LIBS_DEBUG}>
    $<$<CONFIG:Release>:${WWISE_LIBS_RELEASE}>

    # Windows system libs Wwise depends on
    Ws2_32
    Winmm
    Ole32
    Shell32
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${WWISE_AUDIO_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${WWISE_AUDIO_PRIVATE_HEADERS} ${WWISE_AUDIO_PRIVATE_SOURCES})