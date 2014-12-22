add_definitions(-DTRANSLATION_DOMAIN=\"kdevquickopen\")
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

kdevplatform_add_plugin(kdevquickopen JSON kdevquickopen.json SOURCES ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen KF5::IconThemes KF5::GuiAddons KF5::TextEditor KDev::Language KDev::Interfaces KDev::Project KDev::Util)

########### install files ###############

install(FILES kdevquickopen.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevquickopen)
