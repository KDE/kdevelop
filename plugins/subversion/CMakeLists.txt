
project(KDevSubversionPlugin)

add_subdirectory(tests)
add_subdirectory(icons)

include_directories( ${KDevSubversionPlugin_SOURCE_DIR}
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

ki18n_wrap_ui(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
add_library(kdevsubversion MODULE ${kdevsubversion_PART_SRCS} ${kdevsubversion_JOB_SRCS} ${kdevsubversion_WRAPPER_SRCS})

target_link_libraries(kdevsubversion
                    ${SUBVERSION_LIBRARIES}
                    KF5::KIOCore
                    KF5::TextEditor
                    KF5::ThreadWeaver
                    KF5::Parts
                    KDev::Interfaces
                    KDev::Vcs
                    KDev::OutputView
                    KDev::Project
                    KDev::Language
                    )

install(TARGETS kdevsubversion DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############
configure_file(kdevsubversion.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevsubversion.desktop)
kcoreaddons_desktop_to_json(kdevsubversion ${CMAKE_CURRENT_BINARY_DIR}/kdevsubversion.desktop)
