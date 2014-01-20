
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
    documentationquickopenprovider.cpp
    expandingtree/expandingdelegate.cpp
    expandingtree/expandingtree.cpp
    expandingtree/expandingwidgetmodel.cpp
)

set (
    kdevquickopen_UI
    quickopen.ui
)

qt5_wrap_ui(kdevquickopen_PART_SRCS ${kdevquickopen_UI})
add_library(kdevquickopen MODULE ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen kdevplatformlanguage kdevplatforminterfaces kdevplatformproject KF5::TextEditor kdevplatformutil)

install(TARGETS kdevquickopen DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevquickopen.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevquickopen.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevquickopen.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevquickopen.rc DESTINATION ${DATA_INSTALL_DIR}/kdevquickopen)
