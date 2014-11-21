
find_package(LibKompareDiff2 REQUIRED)
add_definitions( ${KDE4_ENABLE_EXCEPTIONS})
include_directories(${LIBKOMPAREDIFF2_INCLUDE_DIR})

set(patchreview_PART_SRCS
    patchreview.cpp
    patchhighlighter.cpp
    patchreviewtoolview.cpp
    localpatchsource.cpp
    standardpatchexport.cpp
)

ki18n_wrap_ui(patchreview_PART_SRCS patchreview.ui localpatchwidget.ui)


add_library(kdevpatchreview MODULE ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview KF5::IconThemes KF5::TextEditor KF5::Parts KF5::KDELibs4Support KDev::Interfaces KDev::Util KDev::Language KDev::Vcs KDev::Sublime ${LIBKOMPAREDIFF2_LIBRARIES})

install(TARGETS kdevpatchreview DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})


########### install files ###############

configure_file(kdevpatchreview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevpatchreview.desktop)
kcoreaddons_desktop_to_json(kdevpatchreview ${CMAKE_CURRENT_BINARY_DIR}/kdevpatchreview.desktop)
install(FILES kdevpatchreview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevpatchreview)
