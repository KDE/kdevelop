add_subdirectory(declarative)

set(welcomepage_SRCS
    welcomepageplugin.cpp
    welcomepagedocument.cpp sessionsmodel.cpp welcomepageview.cpp uihelper.cpp)

qt5_add_resources(welcomepage_SRCS welcomepage.qrc)

add_library(kdevwelcomepage MODULE ${welcomepage_SRCS})

target_link_libraries(kdevwelcomepage
    KDev::Interfaces
    KDev::Sublime
    KDev::Shell
    KDev::Project
    Qt5::QuickWidgets
    KF5::Declarative
    KF5::KDELibs4Support
)

install(TARGETS kdevwelcomepage DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

configure_file(kdevwelcomepage.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevwelcomepage.desktop)
kcoreaddons_desktop_to_json(kdevwelcomepage ${CMAKE_CURRENT_BINARY_DIR}/kdevwelcomepage.desktop)
