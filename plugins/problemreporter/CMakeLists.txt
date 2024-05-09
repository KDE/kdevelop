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
qt_add_resources(kdevproblemreporter_PART_SRCS kdevproblemreporter.qrc)
kdevplatform_add_plugin(kdevproblemreporter SOURCES ${kdevproblemreporter_PART_SRCS})
target_link_libraries(kdevproblemreporter KF6::TextEditor KF6::Parts KF6::KIOWidgets KDev::Language KDev::Interfaces KDev::Util KDev::Project KDev::Shell)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
