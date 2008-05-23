
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9513 )

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

set(kdevcvs_PART_SRCS
    cvsplugin.cpp
    cvsmainview.cpp
    cvsgenericoutputview.cpp
    cvsjob.cpp
    cvsproxy.cpp
    diffoptionsdialog.cpp
    editorsview.cpp
    logview.cpp
    annotateview.cpp
    commitdialog.cpp
    updateoptionsdialog.cpp
    importmetadatawidget.cpp
    importdialog.cpp
    checkoutdialog.cpp
#   cvsfileinfoprovider.cpp
)

set(kdevcvs_PART_UI
    cvsmainview.ui
    cvsgenericoutputview.ui
    diffoptionsdialog.ui
    editorsview.ui
    logview.ui
    annotateview.ui
    commitdialog.ui
    updateoptionsdialog.ui
    importmetadatawidget.ui
    checkoutdialog.ui
)

kde4_add_ui_files(kdevcvs_PART_SRCS ${kdevcvs_PART_UI})
kde4_add_plugin(kdevcvs ${kdevcvs_PART_SRCS})

target_link_libraries(kdevcvs
    ${KDE4_KDECORE_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformproject
)

install(TARGETS kdevcvs DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevcvs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcvs.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcvs )
