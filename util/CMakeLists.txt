add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9508 )

########### next target ###############

set(kdevplatformutil_LIB_SRCS
    autoorientedsplitter.cpp
    formattinghelpers.cpp
    richtextpushbutton.cpp
    kdevstringhandler.cpp
    focusedtreeview.cpp
    processlinemaker.cpp
    commandexecutor.cpp
    environmentconfigurebutton.cpp
    environmentselectionwidget.cpp
    environmentselectionmodel.cpp
    environmentgrouplist.cpp
    activetooltip.cpp
    executecompositejob.cpp
    fileutils.cpp
    shellutils.cpp
    sequentiallyrunjobs.cpp
    multilevellistview.cpp
    placeholderitemproxymodel.cpp
    projecttestjob.cpp
    path.cpp
)

set (kdevplatformutil_LIB_UI
    runoptions.ui
)

if(NOT WIN32)
    add_subdirectory(dbus_socket_transformer)
endif(NOT WIN32)

add_subdirectory(duchainify)

add_subdirectory(tests)

kde4_add_ui_files(kdevplatformutil_LIB_SRCS ${kdevplatformutil_LIB_US})
kde4_add_library(kdevplatformutil SHARED ${kdevplatformutil_LIB_SRCS})
target_link_libraries(kdevplatformutil LINK_PUBLIC
    ${KDE4_KDEUI_LIBS}
)
target_link_libraries(kdevplatformutil LINK_PRIVATE
    ${KDE4_KCMUTILS_LIBRARY}
    kdevplatforminterfaces
)
set_target_properties(kdevplatformutil PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformutil EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdevplatform_shell_environment.sh DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install( FILES kdev_format_source DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)


########### install files ###############

install( FILES
    autoorientedsplitter.h
    formattinghelpers.h
    richtextpushbutton.h
    kdevstringhandler.h
    ksharedobject.h
    focusedtreeview.h
    activetooltip.h
    processlinemaker.h
    commandexecutor.h
    utilexport.h
    environmentconfigurebutton.h
    environmentselectionwidget.h
    environmentgrouplist.h
    pushvalue.h
    kdevvarlengtharray.h
    embeddedfreetree.h
    executecompositejob.h
    convenientfreelist.h
    spinlock.h
    fileutils.h
    sequentiallyrunjobs.h
    multilevellistview.h
    placeholderitemproxymodel.h
    projecttestjob.h
    path.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util COMPONENT Devel)
