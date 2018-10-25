add_library(kdevsvncpp STATIC
    kdevsvncpp/apr.cpp
    kdevsvncpp/client_annotate.cpp
    kdevsvncpp/client_cat.cpp
    kdevsvncpp/client.cpp
    kdevsvncpp/client_diff.cpp
    kdevsvncpp/client_ls.cpp
    kdevsvncpp/client_modify.cpp
    kdevsvncpp/client_property.cpp
    kdevsvncpp/client_status.cpp
    kdevsvncpp/context.cpp
    kdevsvncpp/datetime.cpp
    kdevsvncpp/dirent.cpp
    kdevsvncpp/entry.cpp
    kdevsvncpp/exception.cpp
    kdevsvncpp/info.cpp
    kdevsvncpp/log_entry.cpp
    kdevsvncpp/path.cpp
    kdevsvncpp/pool.cpp
    kdevsvncpp/property.cpp
    kdevsvncpp/revision.cpp
    kdevsvncpp/status.cpp
    kdevsvncpp/status_selection.cpp
    kdevsvncpp/targets.cpp
    kdevsvncpp/url.cpp
    kdevsvncpp/wc.cpp
)
set_property(TARGET kdevsvncpp PROPERTY POSITION_INDEPENDENT_CODE ON)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_target_compile_flag_if_supported(kdevsvncpp PRIVATE "-Wno-documentation")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_target_compile_flag_if_supported(kdevsvncpp PRIVATE "-Wno-cast-align")
    add_target_compile_flag_if_supported(kdevsvncpp PRIVATE "-Wno-zero-as-null-pointer-constant")
endif()
target_include_directories(kdevsvncpp
    PRIVATE kdevsvncpp
)
target_include_directories(kdevsvncpp SYSTEM
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${SUBVERSION_INCLUDE_DIRS}
)
target_link_libraries(kdevsvncpp
    ${SUBVERSION_LIBRARIES}
)
