
project(KDevSubversionPlugin)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} -DKDE_DEFAULT_DEBUG_AREA=9510 )

include_directories( ${KDevSubversionPlugin_SOURCE_DIR}
                     ${KDE4_INCLUDES}
                     ${KDE4_INCLUDE_DIR}/threadweaver
                     ${KDEVPLATFORM_INCLUDE_DIR}
                     ${SUBVERSION_INCLUDE_DIRS}
                     ${KDevSubversionPlugin_BINARY_DIR}
)


########### next target ###############

set(kdevsubversion_WRAPPER_SRCS
    kdevsvncpp/apr.cpp
    kdevsvncpp/client_annotate.cpp
    kdevsvncpp/client_cat.cpp
    kdevsvncpp/client.cpp
    kdevsvncpp/client_diff.cpp
    kdevsvncpp/client_ls.cpp
    kdevsvncpp/client_modify.cpp
    kdevsvncpp/client_property.cpp
    kdevsvncpp/client_status.cpp
    kdevsvncpp/context.cpp
    kdevsvncpp/datetime.cpp
    kdevsvncpp/dirent.cpp
    kdevsvncpp/entry.cpp
    kdevsvncpp/exception.cpp
    kdevsvncpp/info.cpp
    kdevsvncpp/log_entry.cpp
    kdevsvncpp/path.cpp
    kdevsvncpp/pool.cpp
    kdevsvncpp/property.cpp
    kdevsvncpp/revision.cpp
    kdevsvncpp/status.cpp
    kdevsvncpp/status_selection.cpp
    kdevsvncpp/targets.cpp
    kdevsvncpp/url.cpp
    kdevsvncpp/wc.cpp
)

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
    svnimportjob.cpp
    svncheckoutjob.cpp
)

set(kdevsubversion_PART_SRCS
    kdevsvnplugin.cpp
    svnssldialog.cpp
    svncommitdialog.cpp
    svnoutputmodel.cpp
    svnoutputdelegate.cpp
    svnimportmetadatawidget.cpp
    svncheckoutmetadatawidget.cpp
    svnclient.cpp
)

set(kdevsubversion_PART_UI
    ui/ssltrustdialog.ui
    ui/commitdialog.ui
    ui/importmetadatawidget.ui
    ui/checkoutmetadatawidget.ui
)

kde4_add_ui_files(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
kde4_add_plugin(kdevsubversion ${kdevsubversion_PART_SRCS} ${kdevsubversion_JOB_SRCS} ${kdevsubversion_WRAPPER_SRCS})

target_link_libraries(kdevsubversion
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
