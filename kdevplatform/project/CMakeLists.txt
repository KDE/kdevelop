add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

set(KDevPlatformProject_LIB_SRCS
    projectutils.cpp
    projectmodel.cpp
    projectchangesmodel.cpp
    projectconfigskeleton.cpp
    importprojectjob.cpp
    builderjob.cpp
    projectbuildsetmodel.cpp
    projectitemlineedit.cpp
    helper.cpp
    projectproxymodel.cpp
    abstractfilemanagerplugin.cpp
    filemanagerlistjob.cpp
    projectfiltermanager.cpp
    interfaces/iprojectbuilder.cpp
    interfaces/iprojectfilemanager.cpp
    interfaces/ibuildsystemmanager.cpp
    interfaces/iprojectfilter.cpp
    interfaces/iprojectfilterprovider.cpp
    widgets/dependencieswidget.cpp
)

declare_qt_logging_category(KDevPlatformProject_LIB_SRCS
    TYPE LIBRARY
    HEADER debug_project.h
    CATEGORY_BASENAME "project"
)

declare_qt_logging_category(KDevPlatformProject_LIB_SRCS
    TYPE LIBRARY
    HEADER debug_filemanager.h
    CATEGORY_BASENAME "filemanager"
)

ki18n_wrap_ui( KDevPlatformProject_LIB_SRCS widgets/dependencieswidget.ui)

kdevplatform_add_library(KDevPlatformProject SOURCES ${KDevPlatformProject_LIB_SRCS})
target_link_libraries(KDevPlatformProject
PUBLIC
    KDev::Interfaces
    KDev::Util # util/path.h
    KDev::Vcs
PRIVATE
    KDev::Serialization
    KDev::Sublime
    KF6::KIOWidgets
    Qt::Concurrent
)

if(BUILD_TESTING)
    if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
endif()

install(FILES
    interfaces/iprojectbuilder.h
    interfaces/iprojectfilemanager.h
    interfaces/ibuildsystemmanager.h
    interfaces/iprojectfilter.h
    interfaces/iprojectfilterprovider.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/project/interfaces COMPONENT Devel
)

install(FILES
    projectutils.h
    importprojectjob.h
    projectchangesmodel.h
    projectconfigskeleton.h
    projectmodel.h
    projectconfigpage.h
    projectitemlineedit.h
    projectbuildsetmodel.h
    builderjob.h
    helper.h
    abstractfilemanagerplugin.h
    projectfiltermanager.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/project COMPONENT Devel
)

install(FILES
    widgets/dependencieswidget.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/project/widgets COMPONENT Devel
)
