
add_executable(MauEngTests
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Transform/TestTransforms.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Math/TestRotator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Timers/TestTimerManager.cpp")

target_link_libraries(MauEngTests 
    PRIVATE
    Engine
)
target_include_directories(MauEngTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Libs/Doctest")

enable_testing()
add_test(NAME MauEngTests COMMAND MauEngTests)