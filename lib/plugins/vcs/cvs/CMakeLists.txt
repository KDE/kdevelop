
include_directories( ${KDE4_INCLUDES}
	${CMAKE_SOURCE_DIR}/lib 
	${CMAKE_SOURCE_DIR}/lib/interfaces 
	${CMAKE_SOURCE_DIR}/lib/plugins/vcs/interfaces
)


########### next target ###############

set(kdevcvs_PART_SRCS
	cvs_part.cpp
	cvsmainview.cpp
	cvsgenericoutputview.cpp
	cvsjob.cpp
	cvsproxy.cpp
	diffoptionsdialog.cpp
	editorsview.cpp
	logview.cpp
	commitdialog.cpp
	updateoptionsdialog.cpp
)

set(kdevcvs_PART_UI
	cvsmainview.ui
	cvsgenericoutputview.ui
	diffoptionsdialog.ui
	editorsview.ui
	logview.ui
	commitdialog.ui
	updateoptionsdialog.ui
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
