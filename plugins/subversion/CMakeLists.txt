
include_directories( ${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/interfaces
                     ${CMAKE_SOURCE_DIR}/vcs
                     ${CMAKE_SOURCE_DIR}/vcs/interfaces
                     ${CMAKE_SOURCE_DIR}/project
                     ${CMAKE_SOURCE_DIR}/outputview
                     ${SUBVERSION_INCLUDE_DIRS}
)


########### next target ###############

set(kdevsubversion_PART_SRCS
    subversionpart.cpp
    subversionview.cpp
    subversioncore.cpp
    svnmodels.cpp
    svncommitwidgets.cpp
    svnlogviewwidgets.cpp
    svnblamewidgets.cpp
    svnauthdialog.cpp
    svnkjobbase.cpp
    subversionthreads.cpp
    svnrevision.cpp
    interthreadevents.cpp
    svnimportwidgets.cpp
    svncheckoutwidgets.cpp
    svnupdatewidget.cpp
    svndiffwidgets.cpp
    svnrevisionwidget.cpp
    svnstatuswidgets.cpp
    svnrevertwidgets.cpp
    svncopywidgets.cpp
    svnmovewidgets.cpp
    svncatwidgets.cpp
    svnoutputdelegate.cpp
)

set(kdevsubversion_PART_UI
    svnlogviewwidget.ui
    svnblamewidget.ui
    svnlogindialog.ui
    svnssltrustdialog.ui
    svncommitloginputdlg.ui
    uicommitoptiondlg.ui
    uilogviewoptiondlg.ui
    uicheckoutoptiondlg.ui
    svnimportdialog.ui
    uiupdateoptiondlg.ui
    uidiffoptiondlg.ui
    svnrevisionwidget.ui
    uistatusoptiondlg.ui
    uiblameoptiondlg.ui
    uirevertoptiondlg.ui
    uicopyoptiondlg.ui uimoveoptiondlg.ui
    uicatoptiondlg.ui
)

kde4_add_ui_files(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
kde4_add_plugin(kdevsubversion ${kdevsubversion_PART_SRCS})

target_link_libraries(kdevsubversion
                    ${SUBVERSION_LIBRARIES}
                    ${QT_QTDESIGNER_LIBRARY}
                    ${KDE4_KDECORE_LIBS}
                    kdevplatforminterfaces
                    kdevplatformvcs
                    kdevplatformproject
                    kdevplatformoutputview
                    )

install(TARGETS kdevsubversion DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevsubversion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevsubversion.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsubversion )
