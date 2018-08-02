add_definitions(-DTRANSLATION_DOMAIN=\"kdevclangtidy\")

include_directories(
    ${Boost_INCLUDE_DIRS}
)

set(kdevclangtidy_PART_SRCS
    ${kdevclangtidy_LOG_SRCS}

    job.cpp
    plugin.cpp
    checkset.cpp
    utils.cpp

    config/configgroup.cpp
    config/perprojectconfigpage.cpp
    config/clangtidypreferences.cpp

    parsers/clangtidyparser.cpp
    parsers/replacementparser.cpp
)

ki18n_wrap_ui(kdevclangtidy_PART_SRCS
    config/clangtidypreferences.ui
    config/perprojectconfig.ui
)

qt5_add_resources(kdevclangtidy_PART_SRCS
    kdevclangtidy.qrc
)

kconfig_add_kcfg_files(kdevclangtidy_PART_SRCS config/clangtidyconfig.kcfgc)

kdevplatform_add_plugin(kdevclangtidy
    JSON kdevclangtidy.json
    SOURCES ${kdevclangtidy_PART_SRCS}
)

target_link_libraries(kdevclangtidy
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::OutputView
    KDev::Util
    KDev::Shell
    KF5::ItemViews
    KF5::ConfigCore
    KF5::I18n
)
