
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util  )


########### next target ###############

SET(kdevqtdesigner_PART_SRCS
qtdesignerpart.cpp
formwindow_page.cpp
)

kde4_automoc(${kdevqtdesigner_PART_SRCS})

kde4_add_plugin(kdevqtdesigner ${kdevqtdesigner_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevqtdesigner ${KDE4_KDECORE_LIBS} kdevplatform ${QT_QTDESIGNER_LIBRARY} ${QT_QTDESIGNERCOMPONENTS_LIBRARY})
install(TARGETS kdevqtdesigner DESTINATION ${PLUGIN_INSTALL_DIR})
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kdevqtdesigner )

########### install files ###############

install(FILES kdevqtdesigner.desktop DESTINATION  ${SHARE_INSTALL_PREFIX}/services )
install(FILES qtdesignerpart.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevelop )
