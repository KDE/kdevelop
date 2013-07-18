PROJECT(git)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${git_SOURCE_DIR}/cmake)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9525)

add_subdirectory(tests)
add_subdirectory(icons)

set(kdevgit_PART_SRCS
    stashmanagerdialog.cpp
    stashpatchsource.cpp
    gitmessagehighlighter.cpp
    gitclonejob.cpp
    gitplugin.cpp
    gitjob.cpp
)

kde4_add_ui_files(kdevgit_PART_SRCS stashmanagerdialog.ui)
kde4_add_plugin(kdevgit ${kdevgit_PART_SRCS})

target_link_libraries(kdevgit
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_VCS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

install(TARGETS kdevgit DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
configure_file(kdevgit.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgit.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevgit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
