
add_subdirectory(settings)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets
    )


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
target_link_libraries(kdevdocumentview ${KDE4_KDECORE_LIBS} kdevinterfaces kdevwidgets kdevutil ${KDE4_KHTML_LIBS})
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevdocumentview)
install(TARGETS kdevdocumentview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevdocumentview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevdocumentview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevdocumentview )

