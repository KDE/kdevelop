
include_directories( ${CMAKE_SOURCE_DIR}/lib
                     ${CMAKE_SOURCE_DIR}/lib/interfaces
                     ${CMAKE_SOURCE_DIR}/lib/plugins/vcs/interfaces
                     ${CMAKE_SOURCE_DIR}/lib/project
                     ${APU_INCLUDE_DIR}
                     ${APR_INCLUDE_DIR}
                     ${SUBVERSION_INCLUDE_DIR}
)


########### next target ###############

set(kdevsubversion_PART_SRCS
    subversion_part.cpp
    subversion_view.cpp
    subversion_core.cpp
    svn_models.cpp
    svn_commitwidgets.cpp
    svn_logviewwidgets.cpp
    svn_blamewidgets.cpp
    svn_authdialog.cpp
    svnkjobbase.cpp
    subversionthreads.cpp
    subversion_utils.cpp
    interthreadevents.cpp
    svn_importwidgets.cpp
)

set(kdevsubversion_PART_UI
    svnlogviewwidget.ui
    svnblamewidget.ui
    svnlogindialog.ui
    svnssltrustdialog.ui
    svncommitloginputdlg.ui
    uicommit_option_dlg.ui
    svnimportdialog.ui
)

kde4_add_ui_files(kdevsubversion_PART_SRCS ${kdevsubversion_PART_UI})
kde4_automoc(${kdevsubversion_PART_SRCS})
kde4_add_plugin(kdevsubversion ${kdevsubversion_PART_SRCS})

add_definitions (${APR_CPP_FLAGS})
# set_target_properties(kdevsubversion PROPERTIES LINK_FLAGS "${APR_EXTRA_LIBFLAGS} ${APU_EXTRA_LIBFLAGS}")
target_link_libraries(kdevsubversion
                    ${SUBVERSION_ALL_LIBS}
                    ${APR_EXTRA_LIBFLAGS}
                    ${APU_EXTRA_LIBFLAGS}
                    ${QT_QTDESIGNER_LIBRARY}
                    ${KDE4_KDECORE_LIBS}
                    kdevplatforminterfaces
                    kdevplatformvcs
                    kdevplatformproject
                    )

install(TARGETS kdevsubversion DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevsubversion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevsubversion.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsubversion )
