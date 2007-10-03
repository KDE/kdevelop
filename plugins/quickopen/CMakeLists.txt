
include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/language
    ${CMAKE_SOURCE_DIR}/language/interfaces
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/shell
)


########### next target ###############

set(kdevquickopen_PART_SRCS
    quickopen_part.cpp
    quickopenmodel.cpp
    expandingtree/expandingdelegate.cpp
    expandingtree/expandingtree.cpp
    expandingtree/expandingwidgetmodel.cpp
    expandingtree/kateexpandingtree.cpp
)

set (
    kdevquickopen_UI
    quickopen.ui
)

kde4_add_ui_files(kdevquickopen_PART_SRCS ${kdevquickopen_UI})
kde4_add_plugin(kdevquickopen ${kdevquickopen_PART_SRCS})
target_link_libraries(kdevquickopen kdevplatformlanguage kdevplatforminterfaces)

install(TARGETS kdevquickopen DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevquickopen.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevquickopen.rc DESTINATION ${DATA_INSTALL_DIR}/kdevquickopen)
