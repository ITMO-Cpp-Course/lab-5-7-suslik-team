add_library(index_transaction_core STATIC
    src/update_transaction.cpp
    src/index_store.cpp
    src/result.cpp
)

target_link_libraries(index_transaction_core PRIVATE project_warnings in_memory_index_core)
if (TARGET project_sanitizers)
    target_link_libraries(index_transaction_core PRIVATE project_sanitizers)
endif ()

target_include_directories(index_transaction_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)