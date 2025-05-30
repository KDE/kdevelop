add_definitions(-DTRANSLATION_DOMAIN=\"kdevfiletemplates\")
set(kdevfiletemplates_PART_SRCS
    templatepreview.cpp
    templatepreviewtoolview.cpp
    filetemplateprovider.cpp
    filetemplatesplugin.cpp

    ipagefocus.cpp
    classidentifierpage.cpp
    classmemberspage.cpp
    defaultcreateclasshelper.cpp
    licensepage.cpp
    outputpage.cpp
    overridespage.cpp
    templateclassassistant.cpp
    templateoptionspage.cpp
    templateselectionpage.cpp
    testcasespage.cpp
)

declare_qt_logging_category(kdevfiletemplates_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_FILETEMPLATES
    CATEGORY_BASENAME "filetemplates"
)

ki18n_wrap_ui(kdevfiletemplates_PART_SRCS
    templatepreviewtoolview.ui
    ui/licensechooser.ui
    ui/newclass.ui
    ui/outputlocation.ui
    ui/overridevirtuals.ui
    ui/templateselection.ui
    ui/testcases.ui
)

install(FILES kdevfiletemplates.knsrc  DESTINATION ${KDE_INSTALL_KNSRCDIR})
qt_add_resources(kdevfiletemplates_PART_SRCS kdevfiletemplates.qrc)

kdevplatform_add_plugin(kdevfiletemplates SOURCES ${kdevfiletemplates_PART_SRCS})

target_link_libraries(kdevfiletemplates
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KF6::NewStuffWidgets
    KF6::TextEditor
    KF6::Service
    KF6::KIOWidgets
    KF6::TextWidgets
)

########### install files ###############

install(FILES
licenses/GPL\ v2
licenses/GPL\ v3
licenses/LGPL\ v2
licenses/LGPL\ v3
licenses/Apache\ v2
licenses/BSD
licenses/Boost
licenses/MIT\ X11
licenses/Mozilla\ v1.1
"licenses/LGPL v2+ (KDE)"
"licenses/GPL v2+ (KDE)"
DESTINATION ${KDE_INSTALL_DATADIR}/kdevcodegen/licenses
)


################

set(test_srcs
main.cpp
    templatepreview.cpp

    ipagefocus.cpp
    classidentifierpage.cpp
    classmemberspage.cpp
    defaultcreateclasshelper.cpp
    filetemplateprovider.cpp
    licensepage.cpp
    outputpage.cpp
    overridespage.cpp
    templateclassassistant.cpp
    templateoptionspage.cpp
    templateselectionpage.cpp
    testcasespage.cpp
    debug.cpp
)

if(BUILD_TESTING)
    add_executable(testfiletemplates ${test_srcs})
    add_dependencies(testfiletemplates kdevfiletemplates)
    target_link_libraries(testfiletemplates
        KDev::Interfaces
        KDev::Language
        KDev::Project
        KDev::Util
        KDev::Tests
        KF6::KIOWidgets
        KF6::TextWidgets
        KF6::NewStuffWidgets
    )

    add_subdirectory(tests)
endif()
