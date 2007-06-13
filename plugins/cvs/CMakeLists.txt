add_subdirectory(tests)

include_directories( ${KDE4_INCLUDES}
	${CMAKE_SOURCE_DIR}/interfaces
	${CMAKE_SOURCE_DIR}/vcs
)


########### next target ###############

set(kdevcvs_PART_SRCS
	cvspart.cpp
	cvsmainview.cpp
	cvsgenericoutputview.cpp
	cvsjob.cpp
	cvsproxy.cpp
	diffoptionsdialog.cpp
	editorsview.cpp
	logview.cpp
	commitdialog.cpp
	updateoptionsdialog.cpp
	importdialog.cpp
	checkoutdialog.cpp
	cvsfileinfoprovider.cpp
)

set(kdevcvs_PART_UI
	cvsmainview.ui
	cvsgenericoutputview.ui
	diffoptionsdialog.ui
	editorsview.ui
	logview.ui
	commitdialog.ui
	updateoptionsdialog.ui
	importdialog.ui
	checkoutdialog.ui
)

kde4_add_ui_files(kdevcvs_PART_SRCS ${kdevcvs_PART_UI})
kde4_automoc(${kdevcvs_PART_SRCS})
kde4_add_plugin(kdevcvs ${kdevcvs_PART_SRCS})

target_link_libraries(kdevcvs
	${KDE4_KDECORE_LIBS}
	kdevplatforminterfaces
	kdevplatformvcs
)

install(TARGETS kdevcvs DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevcvs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcvs.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcvs )
