
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/interfaces )


########### next target ###############

set(kdevsnippet_PART_SRCS
    snippetpart.cpp
    snippetview.cpp
    snippetvariablesubst.cpp
    snippetvariablemodel.cpp
    snippetstore.cpp
    snippetrepository.cpp
    snippetfilterproxymodel.cpp
    snippet.cpp
    moverepository.cpp
    editsnippet.cpp
    addrepository.cpp
)
set(kdevsnippet_PART_UI
    snippetview.ui
    snippetvariablesubst.ui
    moverepository.ui
    editsnippet.ui
    addrepository.ui
)


kde4_automoc(${kdevsnippet_PART_SRCS})
kde4_add_ui_files(kdevsnippet_PART_SRCS ${kdevsnippet_PART_UI})
kde4_add_plugin(kdevsnippet ${kdevsnippet_PART_SRCS})
target_link_libraries(kdevsnippet ${KDE4_KDECORE_LIBS} kdevplatforminterfaces)
install( TARGETS kdevsnippet DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kdevsnippet.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

