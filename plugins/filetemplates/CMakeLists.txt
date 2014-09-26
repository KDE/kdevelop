set(kdevfiletemplates_PART_SRCS
    templatepreview.cpp
    templatepreviewtoolview.cpp
    filetemplatesplugin.cpp

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

qt5_wrap_ui(kdevfiletemplates_PART_SRCS
    templatepreviewtoolview.ui
    ui/licensechooser.ui
    ui/newclass.ui
    ui/outputlocation.ui
    ui/overridevirtuals.ui
    ui/templateselection.ui
    ui/testcases.ui
)

add_library(kdevfiletemplates MODULE ${kdevfiletemplates_PART_SRCS})

target_link_libraries(kdevfiletemplates
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KF5::NewStuff
    KF5::TextEditor
    KF5::ItemModels
)

install(TARGETS kdevfiletemplates DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevfiletemplates.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevfiletemplates.desktop)
kcoreaddons_desktop_to_json(kdevfiletemplates ${CMAKE_CURRENT_BINARY_DIR}/kdevfiletemplates.desktop)
install( FILES kdevfiletemplates.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevfiletemplates )
install( FILES kdevfiletemplates.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

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
DESTINATION ${DATA_INSTALL_DIR}/kdevcodegen/licenses
)


################

set(test_srcs
main.cpp
    templatepreview.cpp

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

add_executable(testfiletemplates ${test_srcs})

target_link_libraries(testfiletemplates
    
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    KDev::Tests
    KF5::NewStuff
)
