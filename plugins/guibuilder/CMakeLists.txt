
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${KDE4_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(kdevguibuilder_PART_SRCS
guibuilder_part.cpp
formwindow_page.cpp
)

kde4_automoc(${kdevguibuilder_PART_SRCS})

kde4_add_plugin(kdevguibuilder ${kdevguibuilder_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevguibuilder  ${KDE4_KDECORE_LIBS} kdevinterfaces QtDesigner_debug QtDesignerComponents_debug)

INSTALL_TARGETS(/lib/kde4 kdevguibuilder )
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kdevguibuilder )

########### install files ###############

INSTALL_FILES( /share/services FILES  kdevguibuilder.desktop )
INSTALL_FILES( ${DATA_INSTALL_DIR}/kdevelop FILES guibuilderpart.rc )
