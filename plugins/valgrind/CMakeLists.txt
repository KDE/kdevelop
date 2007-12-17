include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/util
)

########### next target ###############

set(kdevvalgrind_PART_SRCS
    valgrind_dialog.cpp
    valgrindplugin.cpp
    valgrindmodel.cpp
    valgrindcontrol.cpp
)

set(kdevvalgrind_PART_UI
    valgrind_dialog.ui
)

kde4_add_ui_files(kdevvalgrind_PART_SRCS ${kdevvalgrind_PART_UI})

kde4_add_plugin(kdevvalgrind ${kdevvalgrind_PART_SRCS})
target_link_libraries(kdevvalgrind ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} kdevplatform)

install(TARGETS kdevvalgrind DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevvalgrind.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevvalgrind.rc DESTINATION ${DATA_INSTALL_DIR}/kdevvalgrind )
