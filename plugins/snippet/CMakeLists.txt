
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9521 )


########### next target ###############

set(kdevsnippet_PART_SRCS
    snippetplugin.cpp
    legacy/snippetview.cpp
    legacy/snippetstore.cpp
    legacy/snippetrepository.cpp
    legacy/snippetfilterproxymodel.cpp
    legacy/snippetcompletionmodel.cpp
    legacy/snippetcompletionitem.cpp
    legacy/snippet.cpp
    legacy/editrepository.cpp
    legacy/editsnippet.cpp
)
set(kdevsnippet_PART_UI
    legacy/snippetview.ui
    legacy/editrepository.ui
    legacy/editsnippet.ui
)

qt5_wrap_ui(kdevsnippet_PART_SRCS ${kdevsnippet_PART_UI})
add_library(kdevsnippet MODULE ${kdevsnippet_PART_SRCS})
target_link_libraries(kdevsnippet ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF3_LIBS}
kdevplatformlanguage kdevplatforminterfaces)
install( TARGETS kdevsnippet DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
configure_file(kdevsnippet.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevsnippet.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevsnippet.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES legacy/kdevsnippet.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsnippet )
