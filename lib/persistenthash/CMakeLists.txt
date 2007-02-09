include_directories(
 ${CMAKE_SOURCE_DIR}/lib
 ${CMAKE_SOURCE_DIR}/lib/interfaces
 ${CMAKE_SOURCE_DIR}/lib/project
)

########### next target ###############

# Reactivate this when the language support and document stuff is sorted out
# set(persistenthash_PLUGIN_SRCS
#     persistenthash.cpp
# )

# kde4_automoc(${persistenthash_PLUGIN_SRCS})

# kde4_add_plugin(kdevpersistenthash ${persistenthash_PLUGIN_SRCS})
# target_link_libraries(kdevpersistenthash ${KDE4_KDECORE_LIBS} kdevplatforminterfaces ${QT_QTDESIGNER_LIBRARY} )
# install(TARGETS kdevpersistenthash DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install(FILES
    interface/ipersistenthash.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/persistenthash/interface
 )

# install(FILES
#     kdevpersistenthash.desktop
#     DESTINATION ${SERVICETYPES_INSTALL_DIR}
# )
