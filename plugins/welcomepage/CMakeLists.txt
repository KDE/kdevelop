add_definitions(-DTRANSLATION_DOMAIN=\"kdevwelcomepage\")
add_subdirectory(declarative)

set(welcomepage_SRCS
    welcomepageplugin.cpp
    sessionsmodel.cpp
    welcomepageview.cpp
    uihelper.cpp
)
qt5_add_resources(welcomepage_SRCS welcomepage.qrc)
declare_qt_logging_category(welcomepage_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_WELCOMEPAGE
    CATEGORY_BASENAME "welcomepage"
)

kdevplatform_add_plugin(kdevwelcomepage JSON kdevwelcomepage.json SOURCES ${welcomepage_SRCS})
option(WELCOMEPAGE_ENABLE_QMLJSDEBUGGING "Enable the QML debugging infrastructure for the welcomepage plugin" OFF)
if (WELCOMEPAGE_ENABLE_QMLJSDEBUGGING)
    # cf. https://doc.qt.io/qt-5/qtquick-debugging.html#enabling-the-infrastructure
    target_compile_definitions(kdevwelcomepage PRIVATE QT_QML_DEBUG=1)
endif()
target_link_libraries(kdevwelcomepage
    KDev::Interfaces
    KDev::Sublime
    KDev::Shell
    KDev::Project
    Qt5::QuickWidgets
    KF5::Declarative
)
# see https://bugs.launchpad.net/ubuntu/+source/gcc-5/+bug/1568899
if (UNIX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(WARNING "Applying workaround for https://bugs.launchpad.net/ubuntu/+source/gcc-5/+bug/1568899")
    target_link_libraries(kdevwelcomepage gcc_s gcc)
endif()
