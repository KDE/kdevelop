add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9045 )
add_subdirectory(tests)
set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
    qthelp_config_shared.cpp
)

add_library(kdevqthelp MODULE ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp
    ${KDE4_KCMUTILS_LIBS} KF5::KIOWidgets ${KDE4_KDEUI_LIBS} KF5::TextEditor ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES})

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR})

configure_file(kdevqthelp.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqthelp.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqthelp.desktop DESTINATION ${SERVICES_INSTALL_DIR})

### Configuration module for QtHelp plugin

set(kdevqthelp_config_SRCS
    qthelpconfig.cpp
    qthelp_config_shared.cpp
)

qt5_wrap_ui(kdevqthelp_config_SRCS qthelpconfig.ui)

add_library(kdevqthelp_config MODULE ${kdevqthelp_config_SRCS} )

target_link_libraries( kdevqthelp_config ${KDE4_KUTILS_LIBS} KF5::KIOWidgets ${QT_QTHELP_LIBRARY} KF5::NewStuff)

install( TARGETS kdevqthelp_config DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdevqthelp_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevelop-qthelp.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
