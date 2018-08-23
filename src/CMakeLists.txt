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

    config/clangtidyprojectconfigpage.cpp
    config/clangtidypreferences.cpp
    config/checkselection.cpp
    config/checklistfilterproxysearchline.cpp
    config/checklistitemproxystyle.cpp
    config/checklistmodel.cpp
    config/checkgroup.cpp

    parsers/clangtidyparser.cpp
# disable for now:
# CentOS used for appimage does not have string_ref.hpp (Boost >= 1.53) as used by current code
#     parsers/replacementparser.cpp
)

ki18n_wrap_ui(kdevclangtidy_PART_SRCS
    config/clangtidypreferences.ui
    config/clangtidyprojectconfigpage.ui
)

qt5_add_resources(kdevclangtidy_PART_SRCS
    kdevclangtidy.qrc
)

kconfig_add_kcfg_files(kdevclangtidy_PART_SRCS
    config/clangtidyconfig.kcfgc
    config/clangtidyprojectconfig.kcfgc
)

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
if(Qt5_VERSION VERSION_LESS "5.10.0")
target_link_libraries(kdevclangtidy
    KF5::ItemModels
)
endif()

install(FILES org.kde.kdev-clang-tidy.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
