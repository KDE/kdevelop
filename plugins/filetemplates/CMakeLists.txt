add_definitions(-DTRANSLATION_DOMAIN=\"kdevfiletemplates\")
set(kdevfiletemplates_PART_SRCS
    templatepreview.cpp
    templatepreviewtoolview.cpp
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
    debug.cpp
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

qt5_add_resources(kdevfiletemplates_PART_SRCS kdevfiletemplates.qrc)
kdevplatform_add_plugin(kdevfiletemplates JSON kdevfiletemplates.json SOURCES ${kdevfiletemplates_PART_SRCS})

target_link_libraries(kdevfiletemplates
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KF5::NewStuff
    KF5::TextEditor
    KF5::ItemModels
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
    licensepage.cpp
    outputpage.cpp
    overridespage.cpp
    templateclassassistant.cpp
    templateoptionspage.cpp
    templateselectionpage.cpp
    testcasespage.cpp
    debug.cpp
)

add_executable(testfiletemplates ${test_srcs})
add_dependencies(testfiletemplates kdevfiletemplates)

target_link_libraries(testfiletemplates
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KDev::Tests
    KF5::NewStuff
)

add_subdirectory(tests)
