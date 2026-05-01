add_library(inverted_index STATIC
    inverted_index.cpp
)

target_include_directories(inverted_index PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})