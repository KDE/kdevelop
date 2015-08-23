add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

set_package_properties(Qt5WebKitWidgets PROPERTIES
    PURPOSE "QtWebKit, for integrated documentation"
    URL "http://qt-project.org/"
    TYPE REQUIRED)

set(KDevPlatformDocumentation_LIB_SRCS
    standarddocumentationview.cpp
    documentationfindwidget.cpp
    documentationview.cpp
)

ki18n_wrap_ui(KDevPlatformDocumentation_LIB_SRCS documentationfindwidget.ui)
kdevplatform_add_library(KDevPlatformDocumentation SOURCES ${KDevPlatformDocumentation_LIB_SRCS})

target_link_libraries(KDevPlatformDocumentation
        LINK_PUBLIC KDev::Interfaces Qt5::WebKitWidgets)

install(FILES
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/documentation COMPONENT Devel
 )
