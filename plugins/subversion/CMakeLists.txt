
include_directories( ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/interfaces
                     ${CMAKE_SOURCE_DIR}/src/vcs
                     ${CMAKE_SOURCE_DIR}/src/project
                     ${CMAKE_SOURCE_DIR}/src/outputview
                     ${SUBVERSION_INCLUDE_DIRS}
)


########### next target ###############

set(kdevsubversion_PART_SRCS
    subversion_part.cpp
    subversion_view.cpp
    subversion_core.cpp
    svn_models.cpp
    svn_commitwidgets.cpp
    svn_logviewwidgets.cpp
    svn_blamewidgets.cpp
    svn_authdialog.cpp
    svnkjobbase.cpp
    subversionthreads.cpp
    subversion_utils.cpp
    interthreadevents.cpp
    svn_importwidgets.cpp
)

set(kdevsubversion_PART_UI
    svnlogviewwidget.ui
    svnblamewidget.ui
    svnlogindialog.ui
    svnssltrustdialog.ui
    svncommitloginputdlg.ui
    uicommit_option_dlg.ui
    uilogview_option_dlg.ui
    svnimportdialog.ui
)

kde4_add_ui_files(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
kde4_automoc(${kdevsubversion_PART_SRCS})
kde4_add_plugin(kdevsubversion ${kdevsubversion_PART_SRCS})

target_link_libraries(kdevsubversion
                    ${SUBVERSION_LIBRARIES}
                    ${QT_QTDESIGNER_LIBRARY}
                    ${KDE4_KDECORE_LIBS}
                    kdevplatforminterfaces
                    kdevplatformvcs
                    kdevplatformproject
                    )

install(TARGETS kdevsubversion DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevsubversion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevsubversion.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsubversion )
