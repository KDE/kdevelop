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

kde4_add_ui_files(kdevfiletemplates_PART_SRCS
    templatepreviewtoolview.ui
    ui/classmembers.ui
    ui/classoptions.ui
    ui/licensechooser.ui
    ui/newclass.ui
    ui/outputlocation.ui
    ui/overridevirtuals.ui
    ui/templateselection.ui
    ui/testcases.ui
)

kde4_add_plugin(kdevfiletemplates ${kdevfiletemplates_PART_SRCS})

target_link_libraries(kdevfiletemplates
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDE4_KNEWSTUFF3_LIBS}
)

install(TARGETS kdevfiletemplates DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevfiletemplates.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevfiletemplates.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevfiletemplates.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevfiletemplates.rc DESTINATION ${DATA_INSTALL_DIR}/kdevfiletemplates )
install( FILES kdevfiletemplates.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

install(FILES
licenses/GPL\ v2
licenses/GPL\ v3
licenses/LGPL\ v2
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

kde4_add_executable(testfiletemplates ${test_srcs})

target_link_libraries(testfiletemplates
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDE4_KNEWSTUFF3_LIBS}
)
