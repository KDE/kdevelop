########### next target ###############

set(kdevexternalscript_PART_SRCS
    externalscriptplugin.cpp
    externalscriptview.cpp
    externalscriptitem.cpp
    externalscriptjob.cpp
    externalscriptoutputmodel.cpp
    editexternalscript.cpp
    externalscriptdebug.cpp
)

set(kdevexternalscript_PART_UI
    externalscriptview.ui
    editexternalscript.ui
)

kde4_add_ui_files(kdevexternalscript_PART_SRCS ${kdevexternalscript_PART_UI})

kde4_add_plugin(kdevexternalscript ${kdevexternalscript_PART_SRCS})

target_link_libraries(kdevexternalscript
    ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF3_LIBS}
    kdevplatformlanguage kdevplatforminterfaces
    kdevplatformutil kdevplatformoutputview
)

install( TARGETS kdevexternalscript DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kdevexternalscript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevexternalscript.rc DESTINATION ${DATA_INSTALL_DIR}/kdevexternalscript )
