include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/outputview
    ${KDEVPLATFORM_INCLUDE_DIR}/util
    ${KDEVPLATFORM_INCLUDE_DIR}/project
)


########### next target ###############

set(kdevgrepview_PART_SRCS
    grepviewpart.cpp
    grepdialog.cpp
    grepoutputmodel.cpp
)

set(kdevgrepview_PART_UI
    grepwidget.ui
)

kde4_add_ui_files(kdevgrepview_PART_SRCS ${kdevgrepview_PART_UI})
kde4_add_plugin(kdevgrepview ${kdevgrepview_PART_SRCS})
target_link_libraries(kdevgrepview
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY}
)

install(TARGETS kdevgrepview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevgrepview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevgrepview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgrepview )
