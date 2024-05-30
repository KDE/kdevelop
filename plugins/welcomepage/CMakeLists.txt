add_definitions(-DTRANSLATION_DOMAIN=\"kdevwelcomepage\")

set(welcomepage_SRCS
    welcomepageplugin.cpp
    sessionlistmodel.cpp
    welcomepageview.cpp
)

ki18n_wrap_ui(welcomepage_SRCS welcomepageview.ui)

declare_qt_logging_category(welcomepage_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_WELCOMEPAGE
    CATEGORY_BASENAME "welcomepage"
)

kdevplatform_add_plugin(kdevwelcomepage SOURCES ${welcomepage_SRCS})

target_link_libraries(kdevwelcomepage
    KDev::Interfaces
    KDev::Sublime
    KDev::Shell
    KDev::Project
    KF5::IconThemes
)
