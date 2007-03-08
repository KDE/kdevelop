
include_directories(${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/plugins/outputviews/interfaces
    ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_SOURCE_DIR}/lib/project/interfaces
    )


########### next target ###############

set(qmakebuilder_LIB_SRCS
    qmakebuilder.cpp
)

kde4_automoc(${qmakebuilder_LIB_SRCS})
kde4_add_plugin(kdevqmakebuilder ${qmakebuilder_LIB_SRCS})
target_link_libraries(
    kdevqmakebuilder
        ${KDE4_KDECORE_LIBS}
        ${QT_QTDESIGNER_LIBRARY}
        kdevplatforminterfaces
        kdevplatformproject
)

install(TARGETS kdevqmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES iqmakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/qmake )
install( FILES kdevqmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


