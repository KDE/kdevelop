
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

qt5_wrap_ui(patchreview_PART_SRCS patchreview.ui localpatchwidget.ui)


add_library(kdevpatchreview MODULE ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview KF5::TextEditor KF5::Parts kdevplatforminterfaces kdevplatformutil kdevplatformlanguage ${KDEVPLATFORM_VCS_LIBRARIES} sublime ${LIBKOMPAREDIFF2_LIBRARIES})

install(TARGETS kdevpatchreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevpatchreview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevpatchreview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevpatchreview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevpatchreview.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevpatchreview)
