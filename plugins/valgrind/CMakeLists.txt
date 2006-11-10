include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util
    ${KDE3_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevvalgrind_PART_SRCS
    valgrind_dialog.cpp
    valgrind_part.cpp
    valgrindmodel.cpp
    valgrindcontrol.cpp
)

kde4_automoc(${kdevvalgrind_PART_SRCS})

set(kdevvalgrind_PART_UI
    valgrind_dialog.ui
)

kde4_add_ui_files(kdevvalgrind_PART_SRCS ${kdevvalgrind_PART_UI})


kde4_add_plugin(kdevvalgrind ${kdevvalgrind_PART_SRCS})
target_link_libraries(kdevvalgrind ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} ktexteditor kdevplatform)

install(TARGETS kdevvalgrind DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevvalgrind.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevpart_valgrind.rc DESTINATION ${DATA_INSTALL_DIR}/kdevvalgrind )

