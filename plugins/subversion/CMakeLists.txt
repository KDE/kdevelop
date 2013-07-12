
project(KDevSubversionPlugin)

add_subdirectory(tests)
add_subdirectory(icons)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} -DKDE_DEFAULT_DEBUG_AREA=9510 )

include_directories( ${KDevSubversionPlugin_SOURCE_DIR}
                     ${KDE4_INCLUDES}
                     ${KDE4_INCLUDE_DIR}/threadweaver
                     ${KDevPlatform_SOURCE_DIR}/interfaces
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
    svnimportmetadatawidget.cpp
    svncheckoutmetadatawidget.cpp
    svnclient.cpp
    svnlocationwidget.cpp
)

set(kdevsubversion_PART_UI
    ui/ssltrustdialog.ui
    ui/importmetadatawidget.ui
    ui/checkoutmetadatawidget.ui
)

kde4_add_ui_files(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
kde4_add_plugin(kdevsubversion ${kdevsubversion_PART_SRCS} ${kdevsubversion_JOB_SRCS} ${kdevsubversion_WRAPPER_SRCS})

target_link_libraries(kdevsubversion
                    ${SUBVERSION_LIBRARIES}
                    ${KDE4_KDEUI_LIBS}
                    ${KDE4_KIO_LIBS}
                    ${KDE4_KTEXTEDITOR_LIBS}
                    kdevplatforminterfaces
                    kdevplatformvcs
                    kdevplatformoutputview
                    kdevplatformproject
                    kdevplatformlanguage
                    ${KDE4_THREADWEAVER_LIBRARIES}
                    ${KDE4_KPARTS_LIBRARY}
                    )

install(TARGETS kdevsubversion DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
configure_file(kdevsubversion.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevsubversion.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevsubversion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
