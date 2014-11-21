
add_subdirectory( tests )

########### next target ###############

set(kdevquickopen_PART_SRCS
    quickopenplugin.cpp
    quickopenmodel.cpp
    projectfilequickopen.cpp
    duchainitemquickopen.cpp
    declarationlistquickopen.cpp
    projectitemquickopen.cpp
    documentationquickopenprovider.cpp
    expandingtree/expandingdelegate.cpp
    expandingtree/expandingtree.cpp
    expandingtree/expandingwidgetmodel.cpp
)

set (
    kdevquickopen_UI
    quickopen.ui
)

ki18n_wrap_ui(kdevquickopen_PART_SRCS ${kdevquickopen_UI})
add_library(kdevquickopen MODULE ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen KF5::IconThemes KF5::GuiAddons KF5::TextEditor KDev::Language KDev::Interfaces KDev::Project KDev::Util)

install(TARGETS kdevquickopen DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})


########### install files ###############

configure_file(kdevquickopen.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevquickopen.desktop)
kcoreaddons_desktop_to_json(kdevquickopen ${CMAKE_CURRENT_BINARY_DIR}/kdevquickopen.desktop)
install(FILES kdevquickopen.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevquickopen)
