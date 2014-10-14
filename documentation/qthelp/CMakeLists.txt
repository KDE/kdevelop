include_directories(${Qt5Help_INCLUDE_DIRS})

set(kdevqthelp_SRCS
    qthelpplugin.cpp
    qthelpproviderabstract.cpp
    qthelpprovider.cpp
    qthelpdocumentation.cpp
    qthelpqtdoc.cpp
    qthelp_config_shared.cpp
    debug.cpp
)

add_library(kdevqthelp MODULE ${kdevqthelp_SRCS})
target_link_libraries(kdevqthelp
    KF5::KCMUtils KF5::I18n KF5::KIOWidgets KF5::TextEditor Qt5::Help Qt5::WebKitWidgets
    KDev::Language KDev::Documentation KDev::Interfaces)

install(TARGETS kdevqthelp DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

configure_file(kdevqthelp.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqthelp.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqthelp.desktop DESTINATION ${SERVICES_INSTALL_DIR})

### Configuration module for QtHelp plugin

set(kdevqthelp_config_SRCS
    qthelpconfig.cpp
    qthelp_config_shared.cpp
    debug.cpp
)

qt5_wrap_ui(kdevqthelp_config_SRCS
    qthelpconfig.ui
    qthelpconfigeditdialog.ui
)

add_library(kdevqthelp_config MODULE ${kdevqthelp_config_SRCS} )

target_link_libraries( kdevqthelp_config
 KF5::KIOWidgets Qt5::Help KF5::NewStuff KF5::KCMUtils KF5::I18n KF5::KDELibs4Support)

install(TARGETS kdevqthelp_config DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)
configure_file(kdevqthelp_config.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqthelp_config.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqthelp_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevelop-qthelp.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

add_subdirectory(tests)
