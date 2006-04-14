kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE3_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevfileselector_PART_SRCS
    fileselector_part.cpp
    fileselector_widget.cpp
    kactionselector.cpp
    kbookmarkhandler.cpp
)

kde4_automoc(${kdevfileselector_PART_SRCS})
kde4_add_plugin(kdevfileselector ${kdevfileselector_PART_SRCS})
target_link_libraries(kdevfileselector ${KDE4_KDECORE_LIBS} ktexteditor kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevfileselector)
install_targets(${PLUGIN_INSTALL_DIR} kdevfileselector)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevfileselector.desktop)

kde4_footer()
