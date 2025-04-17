add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/Doctest")

add_executable(MauEngTests
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TestMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Transform/TestTransforms.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Math/TestRotator.cpp")

target_link_libraries(MauEngTests 
    PRIVATE
    doctest
    Engine
)

enable_testing()
add_test(NAME MauEngTests COMMAND MauEngTests)