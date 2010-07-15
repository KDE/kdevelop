add_subdirectory(icons)
add_subdirectory(tests)

set(kdevreviewboard_PART_SRCS
    reviewboardplugin.cpp
    reviewpatchdialog.cpp
)
kde4_add_ui_files(kdevreviewboard_PART_SRCS reviewpatch.ui)

kde4_add_plugin(kdevreviewboard ${kdevreviewboard_PART_SRCS})
target_link_libraries(kdevreviewboard ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevreviewboard DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevreviewboard.desktop DESTINATION ${SERVICES_INSTALL_DIR})
