project(grepview)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9001 )


########### next target ###############

set(kdevgrepview_PART_SRCS
    grepviewplugin.cpp
    grepdialog.cpp
    grepoutputmodel.cpp
    grepoutputdelegate.cpp
    grepjob.cpp
    grepfindthread.cpp
    grepoutputview.cpp
    greputil.cpp
)

set(kdevgrepview_PART_UI
    grepwidget.ui
    grepoutputview.ui
)

qt5_wrap_ui(kdevgrepview_PART_SRCS ${kdevgrepview_PART_UI})
add_library(kdevgrepview MODULE ${kdevgrepview_PART_SRCS})
target_link_libraries(kdevgrepview
    KF5::Parts
    KF5::TextEditor
    KF5::Completion
    KF5::TextEditor
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevgrepview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevgrepview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgrepview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevgrepview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevgrepview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevgrepview )

add_subdirectory(tests)
