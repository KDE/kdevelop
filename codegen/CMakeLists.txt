include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

kde4_add_library(kdevclangcodegen SHARED
    simplerefactoring.cpp
)

target_link_libraries(kdevclangcodegen LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(kdevclangcodegen LINK_PUBLIC
    kdevclangduchain
)

install(TARGETS kdevclangcodegen ${INSTALL_TARGETS_DEFAULT_ARGS})
