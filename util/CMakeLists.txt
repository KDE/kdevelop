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
    environmentprofilelist.cpp
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
    KF5::GuiAddons
)
install( FILES kdevplatform_shell_environment.sh DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

add_executable(kdev_format_source kdevformatsource.cpp kdevformatfile.cpp)
ecm_mark_nongui_executable(kdev_format_source)
target_link_libraries(kdev_format_source Qt5::Core)
install(TARGETS kdev_format_source DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES .zshrc PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        DESTINATION ${SHARE_INSTALL_PREFIX}/kdevplatform/shellutils/)

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
    environmentprofilelist.h
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
    texteditorhelpers.h
    ${CMAKE_CURRENT_BINARY_DIR}/utilexport.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/util COMPONENT Devel)
