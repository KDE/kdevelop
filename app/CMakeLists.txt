find_package(KF5Plasma)
set_package_properties(KF5Plasma PROPERTIES
    PURPOSE "Plasma libraries for enabling the plasma addons"
    URL "http://www.kde.org"
    TYPE OPTIONAL)
if(KF5Plasma_FOUND)
    add_subdirectory(plasma)
endif()

find_package(SharedMimeInfo REQUIRED)
find_package(KF5Declarative)

set(kdevelop_SRCS  main.cpp kdevideextension.cpp splash.cpp)
# kde4_add_app_icon(kdevelop_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pics/hi*-app-kdevelop.png")

add_executable(kdevelop ${kdevelop_SRCS} )

target_link_libraries(kdevelop
    
    KDev::Interfaces
    KDev::Shell

    KF5::KDELibs4Support
    KF5::Declarative

    Qt5::QuickWidgets
)

install(TARGETS kdevelop ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES kdevelop! DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

# kde4_add_app_icon(kdevelop_bin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pics/hi*-app-kdevelop.png")

if (Q_WS_MAC)
	#sets MacOSX bundle properties for all targets in this cmake file
	#make those assignments target-dependent when cmake 2.6 is useful on MacOSX
	set(MACOSX_BUNDLE_BUNDLE_NAME KDevelop)
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${KDEVELOP_VERSION_MAJOR}.${KDEVELOP_VERSION_MINOR}")
	set(MACOSX_BUNDLE_COPYRIGHT "KDevelop Team")
endif (Q_WS_MAC)


########### install files ###############

install( PROGRAMS kdevelop.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( PROGRAMS kdevelop_ps.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kdeveloprc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc katepartui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevelop )
install( FILES kdevelop.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR} )
install( FILES kdevelop.xml kdevelopinternal.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
install( FILES splash.qml DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
