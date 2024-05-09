add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

########### next target ###############

set(KDevPlatformUtil_LIB_SRCS
    autoorientedsplitter.cpp
    filesystemhelpers.cpp
    foregroundlock.cpp
    formattinghelpers.cpp
    zoomcontroller.cpp
    kdevstringhandler.cpp
    focusedtreeview.cpp
    processlinemaker.cpp
    commandexecutor.cpp
    environmentselectionwidget.cpp
    environmentselectionmodel.cpp
    environmentprofilelist.cpp
    jobstatus.cpp
    activetooltip.cpp
    kdevcoreaddons/kcompoundjob.cpp
    kdevcoreaddons/ksequentialcompoundjob.cpp
    executecompositejob.cpp
    shellutils.cpp
    multilevellistview.cpp
    objectlist.cpp
    placeholderitemproxymodel.cpp
    projecttestjob.cpp
    widgetcolorizer.cpp
    path.cpp
    texteditorhelpers.cpp
    stack.cpp
    expandablelineedit.cpp
)

if(NOT WIN32)
    add_subdirectory(dbus_socket_transformer)
endif()

if(BUILD_TESTING)
    add_subdirectory(duchainify) # needs KDev::Tests
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

declare_qt_logging_category(KDevPlatformUtil_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "util"
)

kdevplatform_add_library(KDevPlatformUtil SOURCES ${KDevPlatformUtil_LIB_SRCS})
target_link_libraries(KDevPlatformUtil
PUBLIC
    KDev::Interfaces
PRIVATE
    Qt::Core5Compat
    KF6::ItemModels
    KF6::GuiAddons
)
install( FILES kdevplatform_shell_environment.sh DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

add_executable(kdev_format_source kdevformatsource.cpp kdevformatfile.cpp)
ecm_mark_nongui_executable(kdev_format_source)
target_link_libraries(kdev_format_source Qt::Core Qt::Core5Compat)
install(TARGETS kdev_format_source DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES .zshrc PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kdevplatform/shellutils/)

########### install files ###############

install( FILES
    algorithm.h
    autoorientedsplitter.h
    filesystemhelpers.h
    foregroundlock.h
    formattinghelpers.h
    zoomcontroller.h
    wildcardhelpers.h
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
    owningrawpointercontainer.h
    placeholderitemproxymodel.h
    projecttestjob.h
    widgetcolorizer.h
    path.h
    stack.h
    stringviewhelpers.h
    texteditorhelpers.h
    ${CMAKE_CURRENT_BINARY_DIR}/utilexport.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/util COMPONENT Devel)

install( FILES
    kdevcoreaddons/kcompoundjob.h
    kdevcoreaddons/ksequentialcompoundjob.h
    kdevcoreaddons/ksimplesequentialcompoundjob.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/util/kdevcoreaddons COMPONENT Devel)
