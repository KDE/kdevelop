add_subdirectory(3rdparty/qtcreator-libs)
add_subdirectory(duchain)
add_subdirectory(nodejsmodules)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(codecompletion)

declare_qt_logging_category(kdevqmljslanguagesupport_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_QMLJS
    CATEGORY_BASENAME "qmljs"
)
kdevplatform_add_plugin(kdevqmljslanguagesupport JSON kdevqmljs.json SOURCES
    qmljsparsejob.cpp
    qmljshighlighting.cpp
    kdevqmljsplugin.cpp
    navigation/propertypreviewwidget.cpp
    ${kdevqmljslanguagesupport_LOG_SRCS}
)
target_link_libraries(kdevqmljslanguagesupport
    kdevqmljsduchain
    kdevqmljscompletion
    Qt5::Widgets
    Qt5::QuickWidgets
    KF5::Declarative
    KDev::DefinesAndIncludesManager
)

install(DIRECTORY navigation/propertywidgets DESTINATION ${KDE_INSTALL_DATADIR}/kdevqmljssupport)
