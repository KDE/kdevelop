add_subdirectory(libdiff2)

########### next target ###############

add_definitions( -fexceptions)

set(patchreview_PART_SRCS      
    patchreview.cpp
    documenthighlighter.cpp
    localpatchsource.cpp
)

set (patchreview_UI
       patchreview.ui)

kde4_add_ui_files(patchreview_PART_SRCS ${patchreview_UI} )


kde4_add_plugin(kdevpatchreview ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KPARTS_LIBS} kdevpatchreview_diff2 kdevplatforminterfaces ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kdevpatchreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevpatchreview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevpatchreview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevpatchreview)
