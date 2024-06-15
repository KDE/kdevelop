add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(dvcs/tests)
    add_subdirectory(models/tests)
endif()

set(KDevPlatformVcs_UIS
    widgets/vcscommitdialog.ui
    widgets/vcseventwidget.ui
    widgets/vcsdiffwidget.ui
    dvcs/ui/dvcsimportmetadatawidget.ui
    dvcs/ui/branchmanager.ui
)

set(KDevPlatformVcs_LIB_SRCS
    vcsjob.cpp
    vcsrevision.cpp
    vcsannotation.cpp
    vcspluginhelper.cpp
    vcslocation.cpp
    vcsdiff.cpp
    vcsevent.cpp
    vcsstatusinfo.cpp
    widgets/vcsannotationitemdelegate.cpp
    widgets/vcsimportmetadatawidget.cpp
    widgets/vcseventwidget.cpp
    widgets/vcsdiffwidget.cpp
    widgets/vcscommitdialog.cpp
    widgets/vcsdiffpatchsources.cpp
    widgets/vcslocationwidget.cpp
    widgets/standardvcslocationwidget.cpp
    models/vcsannotationmodel.cpp
    models/vcseventmodel.cpp
    models/vcsfilechangesmodel.cpp
    models/vcsitemeventmodel.cpp
    models/brancheslistmodel.cpp
    dvcs/dvcsjob.cpp
    dvcs/dvcsplugin.cpp
    dvcs/dvcsevent.cpp
    dvcs/ui/dvcsimportmetadatawidget.cpp
    dvcs/ui/branchmanager.cpp
    interfaces/ibasicversioncontrol.cpp
    interfaces/icontentawareversioncontrol.cpp
    interfaces/ipatchdocument.cpp
    interfaces/ipatchsource.cpp
)

declare_qt_logging_category(KDevPlatformVcs_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "vcs"
)

ki18n_wrap_ui(KDevPlatformVcs_LIB_SRCS ${KDevPlatformVcs_UIS})
kdevplatform_add_library(KDevPlatformVcs SOURCES ${KDevPlatformVcs_LIB_SRCS})
target_link_libraries(KDevPlatformVcs
PUBLIC
    KDev::OutputView
    KDev::Interfaces
PRIVATE
    KDev::Util
    KF6::KIOWidgets
    KF6::Parts
    KF6::TextWidgets
)

install(FILES
    vcsjob.h
    vcsrevision.h
    vcsannotation.h
    vcsdiff.h
    vcspluginhelper.h
    vcsevent.h
    vcsstatusinfo.h
    vcslocation.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/vcs COMPONENT Devel
)

install(FILES
    widgets/vcsimportmetadatawidget.h
    widgets/vcseventwidget.h
    widgets/vcsdiffwidget.h
    widgets/vcscommitdialog.h
    widgets/vcslocationwidget.h
    widgets/standardvcslocationwidget.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/vcs/widgets COMPONENT Devel
)


install(FILES
    models/vcsannotationmodel.h
    models/vcseventmodel.h
    models/vcsfilechangesmodel.h
    models/vcsitemeventmodel.h
    models/brancheslistmodel.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/vcs/models COMPONENT Devel
)

install(FILES
    interfaces/ibasicversioncontrol.h
    interfaces/icentralizedversioncontrol.h
    interfaces/idistributedversioncontrol.h
    interfaces/ibranchingversioncontrol.h
    interfaces/ibrowsableversioncontrol.h
    interfaces/irepositoryversioncontrol.h
    interfaces/ipatchdocument.h
    interfaces/ipatchsource.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/vcs/interfaces COMPONENT Devel
)

install(FILES
    dvcs/dvcsjob.h
    dvcs/dvcsplugin.h
    dvcs/dvcsevent.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/vcs/dvcs COMPONENT Devel
)

install(FILES
    dvcs/ui/dvcsimportmetadatawidget.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/vcs/dvcs/ui COMPONENT Devel
)
