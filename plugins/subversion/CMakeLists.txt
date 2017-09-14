add_definitions(-DTRANSLATION_DOMAIN=\"kdevsubversion\")

# silence the deprecation warnings
# if someone wants to fix the code, I'd welcome it
# but for now, we won't spend time on it...
add_definitions(-DSVN_DEPRECATED=)

kde_enable_exceptions()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(3rdparty)
add_subdirectory(icons)

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

ecm_qt_declare_logging_category(kdevsubversion_PART_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_SVN
    CATEGORY_NAME "kdevplatform.plugins.svn"
)

set(kdevsubversion_PART_UI
    ui/ssltrustdialog.ui
    ui/importmetadatawidget.ui
    ui/checkoutmetadatawidget.ui
)

ki18n_wrap_ui(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})

kdevplatform_add_plugin(kdevsubversion JSON kdevsubversion.json SOURCES ${kdevsubversion_PART_SRCS} ${kdevsubversion_JOB_SRCS})
target_link_libraries(kdevsubversion
    KF5::KIOCore
    KF5::TextEditor
    KF5::ThreadWeaver
    KF5::Parts
    KDev::Interfaces
    KDev::Vcs
    KDev::OutputView
    KDev::Project
    kdevsvncpp
)