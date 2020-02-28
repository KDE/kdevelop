add_definitions(-DTRANSLATION_DOMAIN=\"kdevproblemreporter\")

########### next target ###############

set(kdevproblemreporter_PART_SRCS
    problemreporterplugin.cpp
    problemtreeview.cpp
    problemhighlighter.cpp
    problemsview.cpp
    #problemnavigationcontext.cpp
    problemreportermodel.cpp
    probleminlinenoteprovider.cpp
)

declare_qt_logging_category(kdevproblemreporter_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_PROBLEMREPORTER
    CATEGORY_BASENAME "problemreporter"
)
qt5_add_resources(kdevproblemreporter_PART_SRCS kdevproblemreporter.qrc)
kdevplatform_add_plugin(kdevproblemreporter JSON kdevproblemreporter.json SOURCES ${kdevproblemreporter_PART_SRCS})
target_link_libraries(kdevproblemreporter KF5::TextEditor KF5::Parts KDev::Language KDev::Interfaces KDev::Util KDev::Project KDev::Shell)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
