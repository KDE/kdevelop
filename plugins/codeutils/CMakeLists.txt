project(codeutils)

########### install target ###############

set(kdevcodeutils_PART_SRCS
    templateselectionpage.cpp
    codeutilsplugin.cpp
    testcaseswidget.cpp
    createtestassistant.cpp
    testoutputpage.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

kde4_add_ui_files(kdevcodeutils_PART_SRCS templateselectionpage.ui)
kde4_add_plugin(kdevcodeutils ${kdevcodeutils_PART_SRCS})

target_link_libraries(kdevcodeutils
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

add_subdirectory(doc_templates)


install(TARGETS kdevcodeutils DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcodeutils.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcodeutils.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcodeutils )