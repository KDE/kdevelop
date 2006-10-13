
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE4_INCLUDE_DIRS} )


########### next target ###############

set(kdevfileselector_PART_SRCS
    fileselector_part.cpp
    fileselector_widget.cpp
    kactionselector.cpp
    kbookmarkhandler.cpp
)

kde4_automoc(kdevfileselector ${kdevfileselector_PART_SRCS})
kde4_add_plugin(kdevfileselector ${kdevfileselector_PART_SRCS})
target_link_libraries(kdevfileselector ${KDE4_KDECORE_LIBS} ktexteditor kdevplatform)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevfileselector)
install(TARGETS kdevfileselector DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevfileselector.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

