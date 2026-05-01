add_library(inverted_index STATIC
    inverted_index.cpp
    document_builder.cpp
    document.cpp
)

target_include_directories(inverted_index PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})