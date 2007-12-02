
project(KDevSubversionPlugin)

#@TODO: Hack to make the exceptions from rapidsvn usable
add_definitions( -fexceptions )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${KDE4_INCLUDES}
                     ${KDE4_INCLUDE_DIR}/threadweaver
                     ${CMAKE_SOURCE_DIR}/interfaces
                     ${CMAKE_SOURCE_DIR}/vcs
                     ${CMAKE_SOURCE_DIR}/vcs/interfaces
                     ${CMAKE_SOURCE_DIR}/project
                     ${CMAKE_SOURCE_DIR}/outputview
                     ${SVNCPP_INCLUDE_DIR}
                     ${SUBVERSION_INCLUDE_DIRS}
)


########### next target ###############

set(kdevsubversion_JOB_SRCS
    svninternaljobbase.cpp
    svnjobbase.cpp
    svncommitjob.cpp
    svnstatusjob.cpp
    svnaddjob.cpp
    svnupdatejob.cpp
    svnrevertjob.cpp
    svnremovejob.cpp
    svninfojob.cpp
    svndiffjob.cpp
    svncatjob.cpp
    svncopyjob.cpp
    svnmovejob.cpp
    svnlogjob.cpp
    svnblamejob.cpp
)

set(kdevsubversion_PART_SRCS
    kdevsvnplugin.cpp
    svnssldialog.cpp
    svncommitdialog.cpp
    svnoutputmodel.cpp
    svnoutputdelegate.cpp
    svnclient.cpp
    vcseventmodel.cpp
    vcsitemeventmodel.cpp
    svnlogwidget.cpp
    vcsannotationmodel.cpp
    svnannotationwidget.cpp
    svndiffwidget.cpp
    svnimportmetadatawidget.cpp
)

set(kdevsubversion_PART_UI
    ui/ssltrustdialog.ui
    ui/commitdialog.ui
    ui/logwidget.ui
    ui/annotationwidget.ui
    ui/diffwidget.ui
    ui/importmetadatawidget.ui
)

kde4_add_ui_files(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
kde4_add_plugin(kdevsubversion ${kdevsubversion_PART_SRCS} ${kdevsubversion_JOB_SRCS})

target_link_libraries(kdevsubversion
                    ${SVNCPP_LIBRARY}
                    ${SUBVERSION_LIBRARIES}
                    ${QT_QTDESIGNER_LIBRARY}
                    ${KDE4_KDEUI_LIBS}
                    ${KDE4_KIO_LIBS}
                    ${KDE4_KTEXTEDITOR_LIBS}
                    kdevplatforminterfaces
                    kdevplatformvcs
                    kdevplatformproject
                    kdevplatformoutputview
                    ${KDE4_THREADWEAVER_LIBRARIES}
                    )

install(TARGETS kdevsubversion DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevsubversion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
