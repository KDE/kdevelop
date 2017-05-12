include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${KDevelop_SOURCE_DIR}/languages/plugins
)

add_definitions(-DQT_CREATOR) # :D
add_definitions(-DQML_BUILD_STATIC_LIB=1)

add_subdirectory(libs)
add_subdirectory(duchain)
add_subdirectory(nodejsmodules)
add_subdirectory(tests)
add_subdirectory(codecompletion)

ecm_qt_declare_logging_category(kdevqmljslanguagesupport_LOG_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_QMLJS
    CATEGORY_NAME "kdevelop.languages.qmljs"
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
)

install(DIRECTORY navigation/propertywidgets DESTINATION ${DATA_INSTALL_DIR}/kdevqmljssupport)
