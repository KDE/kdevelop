
include_directories(${CMAKE_SOURCE_DIR}/buildtools/lib/parsers/qmake 
    ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/project
    )


########### next target ###############

set(kdevqmakeimporter_LIB_SRCS
    kdevqmakeeditor.cpp
    qmakeprojectmodel.cpp
    modelcreator.cpp
)

kde4_automoc(kdevqmakeimporter ${kdevqmakeimporter_LIB_SRCS})
kde4_add_plugin(kdevqmakeimporter ${kdevqmakeimporter_LIB_SRCS})
target_link_libraries(kdevqmakeimporter ${KDE4_KDECORE_LIBS} kdevplatform)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevqmakeimporter)
install(TARGETS kdevqmakeimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevqmakeimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


