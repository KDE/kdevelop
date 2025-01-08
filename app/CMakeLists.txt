find_package(Plasma)
set_package_properties(Plasma PROPERTIES
    PURPOSE "Plasma libraries for enabling the plasma addons"
    URL "https://www.kde.org"
    TYPE OPTIONAL)
if(Plasma_FOUND)
    add_subdirectory(plasma)
endif()

option(KDEVELOP_SINGLE_APP "Use QtSingleApplication as KDevelop's base" OFF)

set(kdevelop_SRCS  main.cpp kdevideextension.cpp)
set_source_files_properties(main.cpp PROPERTIES
    COMPILE_DEFINITIONS "RELEASE_SERVICE_VERSION_STRING=\"${RELEASE_SERVICE_VERSION}\""
)
declare_qt_logging_category(kdevelop_SRCS
    IDENTIFIER APP
    TYPE APP
)

qt_add_resources(kdevelop_SRCS kdevelop.qrc)

#TODO: remove CMAKE_CURRENT_SOURCE_DIR prefix when ECM is fixed
set(kdevelop_PNG_ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/16-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/32-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/48-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/64-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/128-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/256-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/512-apps-kdevelop.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/1024-apps-kdevelop.png
)
set(kdevelop_SVG_ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/sc-apps-kdevelop.svg
)
ecm_install_icons(
    ICONS
        ${kdevelop_PNG_ICONS}
        ${kdevelop_SVG_ICONS}
    DESTINATION ${KDE_INSTALL_ICONDIR}
)
ecm_add_app_icon(kdevelop_SRCS
    ICONS ${kdevelop_PNG_ICONS}
)

qt_add_resources(kdevelop_SRCS kdevelop.qrc)
add_executable(kdevelop ${kdevelop_SRCS} )
target_link_libraries(kdevelop
    KDev::Interfaces
    KDev::Shell
    KDev::Util

    KF6::ConfigWidgets
    KF6::Crash
    KF6::IconThemes

    Qt::DBus
    Qt::Quick
)
IF(APPLE)
    target_link_libraries(kdevelop "-framework CoreFoundation")
ENDIF()
if(KDEVELOP_SINGLE_APP)
    add_subdirectory(3rdparty/qtsingleapplication)
    target_link_libraries(kdevelop qtsingleapplication)
endif()

install(TARGETS kdevelop ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES kdevelop! DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
configure_file(start-kdevelop.py.in ${CMAKE_CURRENT_BINARY_DIR}/start-kdevelop.py @ONLY)

option(INSTALL_KDEVELOP_LAUNCH_WRAPPER "Install a wrapper script that sets required environment variables. Only useful when installing into a separate prefix" OFF)
if(INSTALL_KDEVELOP_LAUNCH_WRAPPER)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/start-kdevelop.py DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
endif()

if (WIN32)
install(FILES windows/kdevelop-msvc.bat DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
endif (WIN32)

if (APPLE)
    # the MacOSX bundle display name property (CFBundleDisplayName) is not currently supported by cmake,
    # so has to be set for all targets in this cmake file
    set(MACOSX_BUNDLE_DISPLAY_NAME KDevelop)
    # We provide our own Info.plist to disable AppNap for KDevelop:
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.KDevelop")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDevelop")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_DISPLAY_NAME "KDevelop")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_INFO_STRING "The KDevelop Integrated Development Environment")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING "KDevelop ${KDevelop_VERSION_MAJOR}.${KDevelop_VERSION_MINOR}")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${KDevelop_VERSION_MAJOR}.${KDevelop_VERSION_MINOR}")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${KDevelop_VERSION_MAJOR}.${KDevelop_VERSION_MINOR}")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_ICON_FILE "kdevelop_SRCS.icns")
    set_target_properties(kdevelop PROPERTIES MACOSX_BUNDLE_COPYRIGHT "KDevelop Team")
endif (APPLE)


########### install files ###############

install( PROGRAMS org.kde.kdevelop.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( PROGRAMS org.kde.kdevelop_ps.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( PROGRAMS org.kde.kdevelop_kdev4.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES kdevelop.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR} )
install( FILES kdevelop.xml DESTINATION ${KDE_INSTALL_MIMEDIR} )
update_xdg_mimetypes( ${KDE_INSTALL_MIMEDIR} )
