add_subdirectory(tests)

include_directories( ${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/vcs
    ${CMAKE_SOURCE_DIR}/vcs/interfaces
    ${CMAKE_SOURCE_DIR}/vcs/models
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/
)


########### next target ###############

set(kdevgit_PART_SRCS
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/dvcsjob.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/commitdialog.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/dvcsplugin.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/dvcsmainview.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/dvcsgenericoutputview.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/importdialog.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/importmetadatawidget.cpp
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/logview.cpp
    gitplugin.cpp
    gitexecutor.cpp
)

set(kdevgit_PART_UI
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/commitdialog.ui
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/cvsgenericoutputview.ui
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/cvsmainview.ui
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/importmetadatawidget.ui
    ${CMAKE_SOURCE_DIR}/vcs/dvcs/logview.ui
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

