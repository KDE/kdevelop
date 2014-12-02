add_definitions(-DTRANSLATION_DOMAIN=\"kdevmanpage\")
set(kdevmanpage_SRCS
    manpagedocumentation.cpp
    manpageplugin.cpp
    manpagemodel.cpp
    manpagedocumentationwidget.cpp
)

add_library(kdevmanpage MODULE ${kdevmanpage_SRCS})
target_link_libraries(kdevmanpage KF5::TextEditor KDev::Language
KDev::Documentation KDev::Interfaces KF5::KCMUtils Qt5::WebKitWidgets)

install(TARGETS kdevmanpage DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevmanpage.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevmanpage.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevmanpage.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
    manpagedocumentation.css
    DESTINATION ${DATA_INSTALL_DIR}/kdevmanpage
)

add_subdirectory(tests)
