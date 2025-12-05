# tests/CMakeLists.txt

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(run_tests ${TEST_SOURCES})

target_include_directories(run_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(run_tests
    PRIVATE
        gtest_main
        grustnify_core
)

target_compile_definitions(run_tests
    PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

add_test(NAME run_tests COMMAND run_tests)
