
########### next target ###############

set(KDevPlatformUtil_LIB_SRCS
    autoorientedsplitter.cpp
    foregroundlock.cpp
    formattinghelpers.cpp
    richtextpushbutton.cpp
    kdevstringhandler.cpp
    focusedtreeview.cpp
    processlinemaker.cpp
    commandexecutor.cpp
    environmentselectionwidget.cpp
    environmentselectionmodel.cpp
    environmentgrouplist.cpp
    activetooltip.cpp
    executecompositejob.cpp
    fileutils.cpp
    shellutils.cpp
    mimetype.cpp
    multilevellistview.cpp
    objectlist.cpp
    placeholderitemproxymodel.cpp
    projecttestjob.cpp
    path.cpp
    debug.cpp
)

set (KDevPlatformUtil_LIB_UI
    runoptions.ui
)

if(NOT WIN32)
    add_subdirectory(dbus_socket_transformer)
endif()

add_subdirectory(duchainify)

add_subdirectory(tests)

ki18n_wrap_ui(KDevPlatformUtil_LIB_SRCS ${KDevPlatformUtil_LIB_US})
add_library(KDevPlatformUtil ${KDevPlatformUtil_LIB_SRCS})
add_library(KDev::Util ALIAS KDevPlatformUtil)

generate_export_header(KDevPlatformUtil EXPORT_FILE_NAME utilexport.h)
target_link_libraries(KDevPlatformUtil
LINK_PUBLIC
    KF5::ConfigWidgets
    KF5::ItemModels
LINK_PRIVATE
    KF5::KCMUtils
    KDev::Interfaces
)
target_include_directories(KDevPlatformUtil INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kdevplatform>")
set_target_properties(KDevPlatformUtil PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION} EXPORT_NAME Util)
install(TARGETS KDevPlatformUtil EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdevplatform_shell_environment.sh DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install( FILES kdev_format_source DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)


########### install files ###############

install( FILES
    autoorientedsplitter.h
    foregroundlock.h
    formattinghelpers.h
    richtextpushbutton.h
    kdevstringhandler.h
    ksharedobject.h
    focusedtreeview.h
    activetooltip.h
    processlinemaker.h
    commandexecutor.h
    environmentselectionwidget.h
    environmentgrouplist.h
    pushvalue.h
    kdevvarlengtharray.h
    embeddedfreetree.h
    executecompositejob.h
    convenientfreelist.h
    spinlock.h
    fileutils.h
    mimetype.h
    multilevellistview.h
    objectlist.h
    placeholderitemproxymodel.h
    projecttestjob.h
    path.h
    ${CMAKE_CURRENT_BINARY_DIR}/utilexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util COMPONENT Devel)
