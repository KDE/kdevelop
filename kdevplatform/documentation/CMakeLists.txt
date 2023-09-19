add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

find_package(Qt${KF_MAJOR_VERSION}WebEngineWidgets CONFIG)
if(TARGET Qt${KF_MAJOR_VERSION}::WebEngineWidgets)
    set_package_properties(Qt${KF_MAJOR_VERSION}WebEngineWidgets PROPERTIES
        PURPOSE "QtWebEngine, for integrated documentation"
        URL "https://qt.io/"
        TYPE REQUIRED)
else()
    find_package(Qt${KF_MAJOR_VERSION}WebKitWidgets CONFIG)
    set_package_properties(Qt${KF_MAJOR_VERSION}WebKitWidgets PROPERTIES
        PURPOSE "QtWebKit, for integrated documentation"
        URL "https://qt.io/"
        TYPE REQUIRED)
    set(USE_QTWEBKIT 1)
endif()

set(KDevPlatformDocumentation_LIB_SRCS
    standarddocumentationview.cpp
    documentationfindwidget.cpp
    documentationview.cpp
)

declare_qt_logging_category(KDevPlatformDocumentation_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "documentation"
)

ki18n_wrap_ui(KDevPlatformDocumentation_LIB_SRCS documentationfindwidget.ui)
kdevplatform_add_library(KDevPlatformDocumentation SOURCES ${KDevPlatformDocumentation_LIB_SRCS})

target_link_libraries(KDevPlatformDocumentation
PUBLIC
    KDev::Interfaces
PRIVATE
    KDev::Util
)

if(USE_QTWEBKIT)
    target_link_libraries(KDevPlatformDocumentation PRIVATE Qt${QT_MAJOR_VERSION}::WebKitWidgets)
    target_compile_definitions(KDevPlatformDocumentation PUBLIC -DUSE_QTWEBKIT)
else()
    target_link_libraries(KDevPlatformDocumentation PRIVATE Qt${QT_MAJOR_VERSION}::WebEngineWidgets)
endif()

install(FILES
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/documentation COMPONENT Devel
 )
