add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

# find_package(Qt6WebEngineWidgets CONFIG) # webengine disabled
# set_package_properties(Qt6WebEngineWidgets PROPERTIES
#    PURPOSE "QtWebEngine, for integrated documentation"
#    URL "https://qt.io/"
#    TYPE REQUIRED)

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
    # Qt::WebEngineWidgets # webengine disabled
PRIVATE
    KDev::Util
)

install(FILES
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/documentation COMPONENT Devel
 )
