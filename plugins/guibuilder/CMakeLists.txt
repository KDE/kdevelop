
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util  )


########### next target ###############

SET(kdevguibuilder_PART_SRCS
guibuilder_part.cpp
formwindow_page.cpp
)

kde4_automoc(${kdevguibuilder_PART_SRCS})

kde4_add_plugin(kdevguibuilder ${kdevguibuilder_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevguibuilder  ${KDE4_KDECORE_LIBS} kdevinterfaces QtDesigner_debug QtDesignerComponents_debug)

install(TARGETS kdevguibuilder DESTINATION ${PLUGIN_INSTALL_DIR})
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kdevguibuilder )

########### install files ###############

install(FILES kdevguibuilder.desktop DESTINATION  ${SHARE_INSTALL_PREFIX}/services )
install(FILES guibuilderpart.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevelop )
