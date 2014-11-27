add_definitions(-DTRANSLATION_DOMAIN=\"kdevstandardoutputview\")
add_subdirectory(tests)

########### next target ###############

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
    toolviewdata.cpp
    standardoutputviewmetadata.cpp
)

add_library(kdevstandardoutputview MODULE ${standardoutputview_LIB_SRCS})
target_link_libraries(kdevstandardoutputview
    KDev::Interfaces KDev::Sublime KDev::Util KDev::OutputView)

install(TARGETS kdevstandardoutputview DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

configure_file(kdevstandardoutputview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevstandardoutputview.desktop)
kcoreaddons_desktop_to_json(kdevstandardoutputview ${CMAKE_CURRENT_BINARY_DIR}/kdevstandardoutputview.desktop)
install( FILES kdevstandardoutputview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevstandardoutputview )

