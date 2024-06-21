add_definitions(-DTRANSLATION_DOMAIN=\"kdevclangtidy\")

find_package(ClangTidy QUIET)
set_package_properties(ClangTidy PROPERTIES
    DESCRIPTION "A clang-based C++ “linter” tool"
    URL "https://clang.llvm.org/extra/clang-tidy/"
    TYPE RUNTIME
)

declare_qt_logging_category(kdevclangtidy_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_CLANGTIDY
    CATEGORY_BASENAME "clangtidy"
)

set(kdevclangtidy_PART_SRCS
    ${kdevclangtidy_LOG_SRCS}

    analyzer.cpp
    job.cpp
    plugin.cpp
    checkset.cpp
    checksetselection.cpp
    checksetselectionlock.cpp
    checksetselectionmanager.cpp

    config/clangtidyprojectconfigpage.cpp
    config/clangtidypreferences.cpp
    config/checkselection.cpp
    config/checklistfilterproxysearchline.cpp
    config/checklistitemproxystyle.cpp
    config/checklistmodel.cpp
    config/checkgroup.cpp
    config/checksetselectioncombobox.cpp
    config/checksetselectionlistmodel.cpp
    config/checksetmanagewidget.cpp
    config/customchecksetconfigproxywidget.cpp

    parsers/clangtidyparser.cpp
    parsers/replacementparser.cpp
)

ki18n_wrap_ui(kdevclangtidy_PART_SRCS
    config/checksetmanagewidget.ui
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
    SOURCES ${kdevclangtidy_PART_SRCS}
)

target_link_libraries(kdevclangtidy
    KDevCompileAnalyzerCommon
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

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
