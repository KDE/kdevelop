add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

find_package(Qt5WebEngineWidgets CONFIG)
if(TARGET Qt5::WebEngineWidgets)
    set_package_properties(Qt5WebEngineWidgets PROPERTIES
        PURPOSE "QtWebEngine, for integrated documentation"
        URL "https://qt.io/"
        TYPE REQUIRED)
else()
    find_package(Qt5WebKitWidgets CONFIG)
    set_package_properties(Qt5WebKitWidgets PROPERTIES
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
    target_link_libraries(KDevPlatformDocumentation PRIVATE Qt5::WebKitWidgets)
    target_compile_definitions(KDevPlatformDocumentation PRIVATE -DUSE_QTWEBKIT)
else()
    target_link_libraries(KDevPlatformDocumentation PRIVATE Qt5::WebEngineWidgets)
endif()

install(FILES
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/documentation COMPONENT Devel
 )
