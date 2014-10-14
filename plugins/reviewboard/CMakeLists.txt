add_subdirectory(icons)
add_subdirectory(tests)

set(kdevreviewboard_PART_SRCS
    reviewboardplugin.cpp
    reviewpatchdialog.cpp
    reviewboardjobs.cpp
    debug.cpp
)
qt5_wrap_ui(kdevreviewboard_PART_SRCS reviewpatch.ui)

add_library(kdevreviewboard MODULE ${kdevreviewboard_PART_SRCS})
# FIXME: needs to be added for utilexport.h (needed because iproject MUST include util/path.h or moc breaks)
target_include_directories(kdevreviewboard PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../util)
target_link_libraries(kdevreviewboard KDev::Interfaces KDev::Vcs KF5::I18n KF5::KDELibs4Support Qt5::Core)

install(TARGETS kdevreviewboard DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
configure_file(kdevreviewboard.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevreviewboard.desktop)
kcoreaddons_desktop_to_json(kdevreviewboard ${CMAKE_CURRENT_BINARY_DIR}/kdevreviewboard.desktop)
