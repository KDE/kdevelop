#add_subdirectory(libdiff2)

########### next target ###############

add_definitions( ${KDE4_ENABLE_EXCEPTIONS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/settings)

set(patchreview_PART_SRCS
    patchreview.cpp
    patchhighlighter.cpp
    patchreviewtoolview.cpp
    localpatchsource.cpp
    standardpatchexport.cpp
    settings/diffsettings.cpp
    settings/settingsbase.cpp 

    libdiff2/kompareprocess.cpp
    libdiff2/komparemodellist.cpp
    libdiff2/diffmodellist.cpp
    libdiff2/diffmodel.cpp
    libdiff2/difference.cpp
    libdiff2/diffhunk.cpp
    libdiff2/parser.cpp
    libdiff2/parserbase.cpp
    libdiff2/cvsdiffparser.cpp
    libdiff2/diffparser.cpp
    libdiff2/perforceparser.cpp
    libdiff2/stringlistpair.cpp
)

set (patchreview_UI
    patchreview.ui)

kde4_add_ui_files(patchreview_PART_SRCS ${patchreview_UI} )


kde4_add_plugin(kdevpatchreview ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KPARTS_LIBS} kdevplatforminterfaces kdevplatformutil kdevplatformlanguage ${KDEVPLATFORM_VCS_LIBRARIES} sublime)

install(TARGETS kdevpatchreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevpatchreview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevpatchreview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevpatchreview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevpatchreview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevpatchreview)

add_subdirectory(libdiff2)
