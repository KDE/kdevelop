kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
    ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevdocumentview_PART_SRCS
    kdevdocumentview.cpp
    kdevdocumentviewdelegate.cpp
    kdevdocumentview_part.cpp
    kdevdocumentmodel.cpp
    kdevdocumentselection.cpp
)

kde4_automoc(${kdevdocumentview_PART_SRCS})
kde4_add_plugin(kdevdocumentview ${kdevdocumentview_PART_SRCS})
target_link_libraries(kdevdocumentview ${QT_AND_KDECORE_LIBS} kdevinterfaces kdevwidgets kdevutil khtml)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevdocumentview)
install_targets(${PLUGIN_INSTALL_DIR} kdevdocumentview)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevdocumentview.desktop)
install_files(${DATA_INSTALL_DIR}/kdevdocumentview FILES kdevdocumentview.rc)

kde4_footer()
