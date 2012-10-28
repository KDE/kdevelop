
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9520 )

add_subdirectory( tests )

########### next target ###############

set(kdevquickopen_PART_SRCS
    quickopenplugin.cpp
    quickopenmodel.cpp
    projectfilequickopen.cpp
    duchainitemquickopen.cpp
    declarationlistquickopen.cpp
    projectitemquickopen.cpp
    customlistquickopen.cpp
    expandingtree/expandingdelegate.cpp
    expandingtree/expandingtree.cpp
    expandingtree/expandingwidgetmodel.cpp
)

set (
    kdevquickopen_UI
    quickopen.ui
)

kde4_add_ui_files(kdevquickopen_PART_SRCS ${kdevquickopen_UI})
kde4_add_plugin(kdevquickopen ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen kdevplatformlanguage kdevplatforminterfaces kdevplatformproject ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} kdevplatformutil)

install(TARGETS kdevquickopen DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevquickopen.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevquickopen.rc DESTINATION ${DATA_INSTALL_DIR}/kdevquickopen)
