include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(kdevplatformvcs_UIS
    widgets/vcsannotationwidget.ui
    widgets/vcseventwidget.ui
    widgets/vcsdiffwidget.ui
)

set(kdevplatformvcs_LIB_SRCS
    vcsjob.cpp
    vcsrevision.cpp
    vcsannotation.cpp
    vcslocation.cpp
    vcsdiff.cpp
    vcsevent.cpp
    vcsmapping.cpp
    vcsstatusinfo.cpp
    widgets/vcsimportmetadatawidget.cpp
    widgets/vcsannotationwidget.cpp
    widgets/vcseventwidget.cpp
    widgets/vcsdiffwidget.cpp
    models/vcsannotationmodel.cpp
    models/vcseventmodel.cpp
    models/vcsitemeventmodel.cpp
)

kde4_add_ui_files(kdevplatformvcs_LIB_SRCS ${kdevplatformvcs_UIS})
kde4_add_library(kdevplatformvcs SHARED ${kdevplatformvcs_LIB_SRCS})
target_link_libraries(kdevplatformvcs ${KDE4_KDEUI_LIBS} kdevplatforminterfaces )
set_target_properties(kdevplatformvcs PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformvcs DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    vcsexport.h
    vcsjob.h
    vcsrevision.h
    vcsannotation.h
    vcsdiff.h
    vcsevent.h
    vcsstatusinfo.h
    vcsmapping.h
    vcslocation.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs
)

install(FILES
    widgets/vcsimportmetadatawidget.h
    widgets/vcsannotationwidget.h
    widgets/vcseventwidget.h
    widgets/vcsdiffwidget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/widgets
)


install(FILES
    models/vcsannotationmodel.h
    models/vcseventmodel.h
    models/vcsitemeventmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/models
)

install(FILES
    interfaces/ibasicversioncontrol.h
    interfaces/ibranchingversioncontrol.h
    interfaces/ibrowsableversioncontrol.h
    interfaces/idistributedversioncontrol.h
    interfaces/irepositoryversioncontrol.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/interfaces
)
