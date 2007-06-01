
INCLUDE_DIRECTORIES( 
    ${CMAKE_SOURCE_DIR}/lib 
    ${CMAKE_SOURCE_DIR}/lib/sublime
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)


########### next target ###############

SET(kdevqtdesigner_PART_SRCS
qtdesignerplugin.cpp
qtdesignerdocument.cpp
)

kde4_automoc(${kdevqtdesigner_PART_SRCS})

kde4_add_plugin(kdevqtdesigner ${kdevqtdesigner_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevqtdesigner ${KDE4_KDECORE_LIBS} kdevplatforminterfaces ${QT_QTDESIGNER_LIBRARY} ${QT_QTDESIGNERCOMPONENTS_LIBRARY} sublime)
install(TARGETS kdevqtdesigner DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevqtdesigner.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES kdevqtdesigner.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevqtdesigner )
