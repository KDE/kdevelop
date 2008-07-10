add_subdirectory(tests)

include_directories( ${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/vcs
    ${CMAKE_SOURCE_DIR}/vcs/interfaces
    ${CMAKE_SOURCE_DIR}/vcs/models
)


########### next target ###############

set(kdevgit_PART_SRCS
    gitplugin.cpp
    gitjob.cpp
    gitproxy.cpp
    commitdialog.cpp
    gitmainview.cpp
    gitgenericoutputview.cpp
    importdialog.cpp
    importmetadatawidget.cpp
)

set(kdevgit_PART_UI
    commitdialog.ui
    cvsgenericoutputview.ui
    cvsmainview.ui
    importmetadatawidget.ui
)

kde4_add_ui_files(kdevgit_PART_SRCS ${kdevgit_PART_UI})
kde4_add_plugin(kdevgit ${kdevgit_PART_SRCS})


target_link_libraries(kdevgit
    ${KDE4_KDECORE_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformproject
)

install(TARGETS kdevgit DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevgit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevgit.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgit )

