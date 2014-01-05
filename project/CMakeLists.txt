add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9503 )
set(kdevplatformproject_LIB_SRCS
    projectutils.cpp
    projectmodel.cpp
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
)


add_library(kdevplatformproject SHARED ${kdevplatformproject_LIB_SRCS})
target_link_libraries(kdevplatformproject LINK_PUBLIC
    kdevplatforminterfaces
)
target_link_libraries(kdevplatformproject LINK_PRIVATE
    kdevplatforminterfaces
    kdevplatformutil
    kdevplatformlanguage
    ${KDE4_KIO_LIBS}
)
set_target_properties(kdevplatformproject PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformproject EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

add_subdirectory(tests)

########### install files ###############

install(FILES
    interfaces/iprojectbuilder.h
    interfaces/iprojectfilemanager.h
    interfaces/ibuildsystemmanager.h
    interfaces/iprojectfilter.h
    interfaces/iprojectfilterprovider.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/project/interfaces COMPONENT Devel
 )

install(FILES
    projectutils.h
    importprojectjob.h
    projectconfigskeleton.h
    projectmodel.h
    projectkcmodule.h
    projectexport.h
    projectitemlineedit.h
    projectbuildsetmodel.h
    builderjob.h
    helper.h
    abstractfilemanagerplugin.h
    projectfiltermanager.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/project COMPONENT Devel
 )
