include(FetchContent)

FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(catch2)

add_executable(index_tests
        tests.cpp
)

target_link_libraries(index_tests PRIVATE inverted_index Catch2::Catch2WithMain)

target_include_directories(index_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

if (TARGET project_sanitizers)
        target_link_libraries(index_tests PRIVATE project_sanitizers)
endif ()

add_test(NAME index_tests COMMAND index_tests)