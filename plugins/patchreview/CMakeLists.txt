add_definitions(-DTRANSLATION_DOMAIN=\"kdevpatchreview\")
find_package(LibKompareDiff2 REQUIRED)
kde_enable_exceptions()
include_directories(${LIBKOMPAREDIFF2_INCLUDE_DIR})

set(patchreview_PART_SRCS
    patchreview.cpp
    patchhighlighter.cpp
    patchreviewtoolview.cpp
    localpatchsource.cpp
    standardpatchexport.cpp
)

ki18n_wrap_ui(patchreview_PART_SRCS patchreview.ui localpatchwidget.ui)

kdevplatform_add_plugin(kdevpatchreview JSON kdevpatchreview.json SOURCES ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview KF5::IconThemes KF5::TextEditor KF5::Parts KF5::KDELibs4Support KDev::Interfaces KDev::Util KDev::Language KDev::Vcs KDev::Sublime ${LIBKOMPAREDIFF2_LIBRARIES})

########### install files ###############

install(FILES kdevpatchreview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevpatchreview)
