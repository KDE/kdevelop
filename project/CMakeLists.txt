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
generate_export_header(kdevplatformproject EXPORT_FILE_NAME projectexport.h)
# TODO: helper.cpp needs vcsexport.h and outputviewexport.h, but can't link to kdevplatformvcs
target_include_directories(kdevplatformproject PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../vcs/")
target_include_directories(kdevplatformproject PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../outputview/")
target_link_libraries(kdevplatformproject LINK_PUBLIC
    kdevplatforminterfaces
    kdevplatformutil # util/path.h
)
target_link_libraries(kdevplatformproject LINK_PRIVATE
    kdevplatforminterfaces
    kdevplatformlanguage
    KF5::KIOWidgets
    Qt5::Concurrent
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
    projectitemlineedit.h
    projectbuildsetmodel.h
    builderjob.h
    helper.h
    abstractfilemanagerplugin.h
    projectfiltermanager.h
    ${CMAKE_CURRENT_BINARY_DIR}/projectexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/project COMPONENT Devel
 )
