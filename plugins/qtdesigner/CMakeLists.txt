
INCLUDE_DIRECTORIES( 
    ${KDEVPLATFORM_INCLUDE_DIR} 
    ${KDEVPLATFORM_INCLUDE_DIR}/${KDEVPLATFORM_SUBLIME_LIBRARY}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
)


########### next target ###############

SET(kdevqtdesigner_PART_SRCS
qtdesignerplugin.cpp
qtdesignerdocument.cpp
)



kde4_add_plugin(kdevqtdesigner ${kdevqtdesigner_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevqtdesigner ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${QT_QTDESIGNER_LIBRARY} ${QT_QTDESIGNERCOMPONENTS_LIBRARY} ${KDEVPLATFORM_SUBLIME_LIBRARY})
install(TARGETS kdevqtdesigner DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevqtdesigner.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES kdevqtdesigner.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevqtdesigner )
