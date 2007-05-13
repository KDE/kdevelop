
include_directories(${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/util
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    )


########### next target ###############

set(simpleoutputview_LIB_SRCS
    simpleoutputview.cpp
    outputviewcommand.cpp
    outputwidget.cpp
)

kde4_automoc(${simpleoutputview_LIB_SRCS})
kde4_add_plugin(kdevsimpleoutputview ${simpleoutputview_LIB_SRCS})
target_link_libraries(kdevsimpleoutputview ${KDE4_KDECORE_LIBS}
    ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces kdevplatformutil)

install(TARGETS kdevsimpleoutputview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES interfaces/ioutputview.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/outputviews )
install( FILES kdevsimpleoutputview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


