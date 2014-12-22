add_definitions(-DTRANSLATION_DOMAIN=\"kdevreviewboard\")
add_subdirectory(icons)
add_subdirectory(tests)

set(kdevreviewboard_PART_SRCS
    reviewboardplugin.cpp
    reviewpatchdialog.cpp
    reviewboardjobs.cpp
    debug.cpp
)
ki18n_wrap_ui(kdevreviewboard_PART_SRCS reviewpatch.ui)

kdevplatform_add_plugin(kdevreviewboard JSON kdevreviewboard.json SOURCES ${kdevreviewboard_PART_SRCS})

# FIXME: needs to be added for utilexport.h (needed because iproject MUST include util/path.h or moc breaks)
target_include_directories(kdevreviewboard PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../util)
target_link_libraries(kdevreviewboard KDev::Interfaces KDev::Vcs KF5::I18n KF5::KDELibs4Support Qt5::Core)
