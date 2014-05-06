add_subdirectory(dvcs/tests)
add_subdirectory(models/tests)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9509)

set(kdevplatformvcs_UIS
    widgets/vcscommitdialog.ui
    widgets/vcseventwidget.ui
    widgets/vcsdiffwidget.ui
    dvcs/ui/importmetadatawidget.ui
    dvcs/ui/branchmanager.ui
)

set(kdevplatformvcs_LIB_SRCS
    vcsjob.cpp
    vcsrevision.cpp
    vcsannotation.cpp
    vcspluginhelper.cpp
    vcslocation.cpp
    vcsdiff.cpp
    vcsevent.cpp
    vcsstatusinfo.cpp
    widgets/vcsimportmetadatawidget.cpp
    widgets/vcseventwidget.cpp
    widgets/vcsdiffwidget.cpp
    widgets/vcscommitdialog.cpp
    widgets/vcsdiffpatchsources.cpp
    widgets/vcslocationwidget.cpp
    widgets/standardvcslocationwidget.cpp
    widgets/flexibleaction.cpp
    models/vcsannotationmodel.cpp
    models/vcseventmodel.cpp
    models/vcsfilechangesmodel.cpp
    models/vcsitemeventmodel.cpp
    models/projectchangesmodel.cpp
    models/brancheslistmodel.cpp
    dvcs/dvcsjob.cpp
    dvcs/dvcsplugin.cpp
    dvcs/ui/importmetadatawidget.cpp
    dvcs/ui/branchmanager.cpp
    dvcs/ui/revhistory/commitView.cpp
    dvcs/ui/revhistory/commitlogmodel.cpp
    interfaces/ibasicversioncontrol.cpp
    interfaces/icontentawareversioncontrol.cpp
    interfaces/ipatchdocument.cpp
    interfaces/ipatchexporter.cpp
    interfaces/ipatchsource.cpp
)

qt5_wrap_ui(kdevplatformvcs_LIB_SRCS ${kdevplatformvcs_UIS})
add_library(kdevplatformvcs SHARED ${kdevplatformvcs_LIB_SRCS})
target_link_libraries(kdevplatformvcs LINK_PUBLIC
    kdevplatformoutputview
    kdevplatforminterfaces
)
target_link_libraries(kdevplatformvcs LINK_PRIVATE
    KF5::KIOWidgets
    KF5::Parts
    kdevplatformutil
    kdevplatformproject
    kdevplatformlanguage
)
set_target_properties(kdevplatformvcs PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformvcs EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    vcsexport.h
    vcsjob.h
    vcsrevision.h
    vcsannotation.h
    vcsdiff.h
    vcspluginhelper.h
    vcsevent.h
    vcsstatusinfo.h
    vcslocation.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs COMPONENT Devel
)

install(FILES
    widgets/vcsimportmetadatawidget.h
    widgets/vcseventwidget.h
    widgets/vcsdiffwidget.h
    widgets/vcscommitdialog.h
    widgets/vcslocationwidget.h
    widgets/standardvcslocationwidget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/widgets COMPONENT Devel
)


install(FILES
    models/vcsannotationmodel.h
    models/vcseventmodel.h
    models/vcsfilechangesmodel.h
    models/vcsitemeventmodel.h
    models/projectchangesmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/models COMPONENT Devel
)

install(FILES
    interfaces/ibasicversioncontrol.h
    interfaces/icentralizedversioncontrol.h
    interfaces/idistributedversioncontrol.h
    interfaces/ibranchingversioncontrol.h
    interfaces/ibrowsableversioncontrol.h
    interfaces/irepositoryversioncontrol.h
    interfaces/ipatchdocument.h
    interfaces/ipatchexporter.h
    interfaces/ipatchsource.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/interfaces COMPONENT Devel
)

install(FILES
    dvcs/dvcsjob.h
    dvcs/dvcsplugin.h
    dvcs/dvcsevent.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/dvcs COMPONENT Devel
)
