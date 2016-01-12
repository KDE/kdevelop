add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

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
    jobstatus.cpp
    activetooltip.cpp
    executecompositejob.cpp
    shellutils.cpp
    multilevellistview.cpp
    objectlist.cpp
    placeholderitemproxymodel.cpp
    projecttestjob.cpp
    widgetcolorizer.cpp
    path.cpp
    debug.cpp
    texteditorhelpers.cpp
    stack.cpp
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
kdevplatform_add_library(KDevPlatformUtil SOURCES ${KDevPlatformUtil_LIB_SRCS})
target_link_libraries(KDevPlatformUtil
LINK_PUBLIC
    KDev::Interfaces
    KF5::ItemModels
LINK_PRIVATE
    KF5::ConfigWidgets
    KF5::TextEditor
)

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
    jobstatus.h
    pushvalue.h
    kdevvarlengtharray.h
    embeddedfreetree.h
    executecompositejob.h
    convenientfreelist.h
    multilevellistview.h
    objectlist.h
    placeholderitemproxymodel.h
    projecttestjob.h
    widgetcolorizer.h
    path.h
    stack.h
    ${CMAKE_CURRENT_BINARY_DIR}/utilexport.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/util COMPONENT Devel)
