add_definitions(-DTRANSLATION_DOMAIN=\"kdevproblemreporter\")

########### next target ###############

set(kdevproblemreporter_PART_SRCS
    problemreporterplugin.cpp
    problemtreeview.cpp
    problemmodel.cpp
    problemhighlighter.cpp
    watcheddocumentset.cpp
)

kdevplatform_add_plugin(kdevproblemreporter JSON kdevproblemreporter.json SOURCES ${kdevproblemreporter_PART_SRCS})
target_link_libraries(kdevproblemreporter KF5::TextEditor KF5::Parts KDev::Language KDev::Interfaces KDev::Util KDev::Project)

########### install files ###############

install(FILES kdevproblemreporter.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevproblemreporter)
