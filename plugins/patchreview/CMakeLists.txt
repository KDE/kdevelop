find_package(LibKompareDiff2 5.1 CONFIG)
set_package_properties(LibKompareDiff2 PROPERTIES
    PURPOSE "Required for building the patch review plugin."
    TYPE REQUIRED
)

find_package(KF5Purpose CONFIG QUIET)
set_package_properties(KF5Purpose PROPERTIES DESCRIPTION "Support for patch sharing"
                       TYPE OPTIONAL
                      )
if (NOT KF5Purpose_FOUND)
    find_package(KDEExperimentalPurpose QUIET)
    set_package_properties(KDEExperimentalPurpose PROPERTIES DESCRIPTION "EXPERIMENTAL. Support for patch sharing"
                        URL "https://projects.kde.org/projects/playground/libs/purpose"
                        TYPE OPTIONAL
                        )
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kdevpatchreview\")
kde_enable_exceptions()

set(patchreview_PART_SRCS
    patchreview.cpp
    patchhighlighter.cpp
    patchreviewtoolview.cpp
    localpatchsource.cpp
)

declare_qt_logging_category(patchreview_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_PATCHREVIEW
    CATEGORY_BASENAME "patchreview"
)
ki18n_wrap_ui(patchreview_PART_SRCS patchreview.ui localpatchwidget.ui)

qt5_add_resources(patchreview_PART_SRCS kdevpatchreview.qrc)
kdevplatform_add_plugin(kdevpatchreview SOURCES ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview
    KDev::Project
    KDev::Interfaces
    KDev::Util
    KDev::Language
    KDev::Vcs
    KDev::Sublime
    ${LIBKOMPAREDIFF2_LIBRARIES} # from cmake config file, has matching target name, which changed for 5.1
    KF5::IconThemes
    KF5::TextEditor
    KF5::Parts
)
if (LibKompareDiff2_VERSION VERSION_GREATER_EQUAL 5.4.0)
    # KompareDiff2 installs a working version header only since 5.4 (23.08.0)
    # so to work also with old versions instead of the sources including
    # the version header a compile definition flag is used
    target_compile_definitions(kdevpatchreview PRIVATE WITH_KOMPAREDIFF2_5_4_OR_NEWER)
endif()
if (KF5Purpose_FOUND)
    target_compile_definitions(kdevpatchreview PRIVATE WITH_PURPOSE)
    target_link_libraries(kdevpatchreview KF5::PurposeWidgets)
elseif (KDEExperimentalPurpose_FOUND)
    target_compile_definitions(kdevpatchreview PRIVATE WITH_PURPOSE)
    target_link_libraries(kdevpatchreview KDEExperimental::PurposeWidgets)
endif()
