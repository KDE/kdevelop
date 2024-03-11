find_package(KompareDiff2 6.0)
set_package_properties(KompareDiff2 PROPERTIES
    PURPOSE "Required for building the patch review plugin."
    TYPE RECOMMENDED
)
if (NOT KompareDiff2_FOUND)
    return()
endif()

find_package(KF6Purpose CONFIG QUIET)
set_package_properties(KF6Purpose PROPERTIES DESCRIPTION "Support for patch sharing"
                       TYPE OPTIONAL
                      )

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

qt_add_resources(patchreview_PART_SRCS kdevpatchreview.qrc)
kdevplatform_add_plugin(kdevpatchreview SOURCES ${patchreview_PART_SRCS})
target_link_libraries(kdevpatchreview
    KDev::Project
    KDev::Interfaces
    KDev::Util
    KDev::Language
    KDev::Vcs
    KDev::Sublime
    KompareDiff2
    KF6::IconThemes
    KF6::KIOWidgets
    KF6::TextEditor
    KF6::Parts
)
if (KF6Purpose_FOUND)
    target_compile_definitions(kdevpatchreview PRIVATE WITH_PURPOSE)
    target_link_libraries(kdevpatchreview KF6::PurposeWidgets)
endif()
