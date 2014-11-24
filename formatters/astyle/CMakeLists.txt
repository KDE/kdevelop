include_directories(lib)

set(kdevastyle_PART_SRCS
    astyle_plugin.cpp
    astyle_preferences.cpp
    astyle_formatter.cpp
    astyle_stringiterator.cpp
    debug.cpp
    lib/ASBeautifier.cpp
    lib/ASEnhancer.cpp
    lib/ASFormatter.cpp
    lib/ASResource.cpp
)

set(astyle_preferences_UI
    astyle_preferences.ui
)

ki18n_wrap_ui(kdevastyle_PART_SRCS ${astyle_preferences_UI} )
add_library(kdevastyle MODULE ${kdevastyle_PART_SRCS})
target_link_libraries(kdevastyle


    KF5::Parts
    KF5::KIOWidgets
    KF5::TextEditor
    KDev::Interfaces
    KDev::Project
    KDev::Util)

install(TARGETS kdevastyle DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

configure_file(kdevastyle.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevastyle.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevastyle.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

add_subdirectory(tests)
