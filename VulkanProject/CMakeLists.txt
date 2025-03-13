cmake_minimum_required(VERSION 3.27)

# Create the executable target
add_executable(${EXECUTABLE_NAME}
    src/private/main.cpp
)

target_include_directories(${EXECUTABLE_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/public
)

find_package(Vulkan REQUIRED)

target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Libs           # Aggregated target from Libs folder (GLFW, GLM, etc.)
        Vulkan::Vulkan # Vulkan SDK (ensure it is installed and found)
)