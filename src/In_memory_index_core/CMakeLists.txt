add_library(in_memory_index_core STATIC
        src/document.cpp
        src/inverted_index.cpp
        src/document_builder.cpp
)

target_link_libraries(in_memory_index_core PRIVATE project_warnings)
if (TARGET project_sanitizers)
    target_link_libraries(in_memory_index_core PRIVATE project_sanitizers)
endif ()

target_include_directories(in_memory_index_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)