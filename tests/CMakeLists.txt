include(FetchContent)

FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(catch2)

add_executable(resource_tests
        tests.cpp
)

target_include_directories(resource_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/In_memory_index_core/include
)

target_link_libraries(resource_tests PRIVATE in_memory_index_core project_warnings Catch2::Catch2WithMain)
if (TARGET project_sanitizers)
        target_link_libraries(resource_tests PRIVATE project_sanitizers)
endif ()

add_test(NAME resource_tests COMMAND resource_tests)
