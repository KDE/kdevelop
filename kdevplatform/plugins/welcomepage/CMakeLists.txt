add_definitions(-DTRANSLATION_DOMAIN=\"kdevwelcomepage\")
add_subdirectory(declarative)

set(welcomepage_SRCS
    welcomepageplugin.cpp
    sessionsmodel.cpp
    welcomepageview.cpp
    uihelper.cpp)

qt5_add_resources(welcomepage_SRCS welcomepage.qrc)

kdevplatform_add_plugin(kdevwelcomepage JSON kdevwelcomepage.json SOURCES ${welcomepage_SRCS})
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
