
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9522 )

add_subdirectory(tests)

########### next target ###############

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
    toolviewdata.cpp
)

add_library(kdevstandardoutputview MODULE ${standardoutputview_LIB_SRCS})
target_link_libraries(kdevstandardoutputview
    KDev::Interfaces KDev::Sublime KDev::Util KDev::OutputView)

install(TARGETS kdevstandardoutputview DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

configure_file(kdevstandardoutputview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevstandardoutputview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevstandardoutputview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevstandardoutputview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevstandardoutputview )

