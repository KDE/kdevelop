
include_directories(
    ${CMAKE_SOURCE_DIR}/src/interfaces
    ${CMAKE_SOURCE_DIR}/src/util
    ${CMAKE_SOURCE_DIR}/src/outputview
    )


########### next target ###############

set(standardoutputview_LIB_SRCS
    standardoutputview.cpp
    outputwidget.cpp
)

kde4_automoc(${standardoutputview_LIB_SRCS})
kde4_add_plugin(kdevstandardoutputview ${standardoutputview_LIB_SRCS})
target_link_libraries(kdevstandardoutputview ${KDE4_KDECORE_LIBS}
    ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces kdevplatformutil kdevplatformoutputview)

install(TARGETS kdevstandardoutputview DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kdevstandardoutputview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevstandardoutputview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevstandardoutputview )

