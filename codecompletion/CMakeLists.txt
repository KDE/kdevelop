include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

kde4_add_library(kdevclangcodecompletion SHARED
    model.cpp
    context.cpp
    navigationwidget.cpp
)

target_link_libraries(kdevclangcodecompletion LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(kdevclangcodecompletion LINK_PUBLIC
    kdevclangduchain
)

install(TARGETS kdevclangcodecompletion ${INSTALL_TARGETS_DEFAULT_ARGS})
