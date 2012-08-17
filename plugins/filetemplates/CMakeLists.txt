
set(kdevfiletemplates_PART_SRCS
    templatepreview.cpp
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
    templatepreview.ui
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
    ${KDE4_KNEWSTUFF3_LIBS}
)

install(TARGETS kdevfiletemplates DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevfiletemplates.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevfiletemplates.rc DESTINATION ${DATA_INSTALL_DIR}/kdevfiletemplates )
install( FILES kdevfiletemplates.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
