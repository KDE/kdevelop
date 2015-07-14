add_definitions(-DTRANSLATION_DOMAIN=\"kdevproblemreporter\")

########### next target ###############

set(kdevproblemreporter_PART_SRCS
    problemreporterplugin.cpp
    problemtreeview.cpp
    problemhighlighter.cpp
    problemsview.cpp
    problemnavigationcontext.cpp
    problemreportermodel.cpp
)

kdevplatform_add_plugin(kdevproblemreporter JSON kdevproblemreporter.json SOURCES ${kdevproblemreporter_PART_SRCS})
target_link_libraries(kdevproblemreporter KF5::TextEditor KF5::Parts KDev::Language KDev::Interfaces KDev::Util KDev::Project KDev::Shell)

########### install files ###############

install(FILES kdevproblemreporter.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kdevproblemreporter)
