add_subdirectory(profiles)
# add_subdirectory(profileengine)

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/sublime
    ${KDEVPLATFORM_INCLUDE_DIR}/shell
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    )


########### next target ###############

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
    splashscreen.cpp
)



kde4_add_executable(kdevelop ${kdevelop_SRCS})

if (Q_WS_MAC)
	#sets MacOSX bundle properties for all targets in this cmake file
	#make those assignments target-dependent when cmake 2.6 is useful on MacOSX
	set(MACOSX_BUNDLE_BUNDLE_NAME KDevelop)
	set(MACOSX_BUNDLE_BUNDLE_VERSION "4.0")
	set(MACOSX_BUNDLE_COPYRIGHT "KDevelop Team")
endif (Q_WS_MAC)

target_link_libraries(kdevelop ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_SHELL_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTDESIGNER_LIBRARY} )
install(TARGETS kdevelop ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES kdevelop.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kdeveloprc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc kdevelop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
