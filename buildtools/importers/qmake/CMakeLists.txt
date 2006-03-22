kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/buildtools/lib/parsers/qmake 
    ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevqmakeimporter_LIB_SRCS
    kdevqmakeeditor.cpp
    qmakeprojectmodel.cpp
    modelcreator.cpp
)

kde4_automoc(${kdevqmakeimporter_LIB_SRCS})
kde4_add_plugin(kdevqmakeimporter ${kdevqmakeimporter_LIB_SRCS})
target_link_libraries(kdevqmakeimporter ${KDE4_KDECORE_LIBS} kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevqmakeimporter)
install_targets(${PLUGIN_INSTALL_DIR} kdevqmakeimporter)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevqmakeimporter.desktop)


kde4_footer()
