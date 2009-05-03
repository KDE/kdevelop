project(valgrind)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9002 )

########### next target ###############

set(kdevvalgrind_PART_SRCS
    valgrindplugin.cpp
    valgrindmodel.cpp
    valgrindcontrol.cpp
    valgrindwidget.cpp
    valgrindtree.cpp
    valgrindconfig.cpp
#modeltest.cpp
)

set(kdevvalgrind_PART_UI valgrindsettings.ui)

kde4_add_ui_files(kdevvalgrind_PART_SRCS ${kdevvalgrind_PART_UI})

kde4_add_plugin(kdevvalgrind ${kdevvalgrind_PART_SRCS})
target_link_libraries(kdevvalgrind
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${QT_QTNETWORK_LIBRARY})

install(TARGETS kdevvalgrind DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevvalgrind.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevvalgrind.rc DESTINATION ${DATA_INSTALL_DIR}/kdevvalgrind )
