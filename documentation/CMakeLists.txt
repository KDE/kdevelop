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
add_library(KDevPlatformDocumentation ${KDevPlatformDocumentation_LIB_SRCS})
add_library(KDev::Documentation ALIAS KDevPlatformDocumentation)
generate_export_header(KDevPlatformDocumentation EXPORT_FILE_NAME documentationexport.h)

install(TARGETS KDevPlatformDocumentation EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries(KDevPlatformDocumentation
        LINK_PUBLIC KDev::Interfaces Qt5::WebKitWidgets)

set_target_properties(KDevPlatformDocumentation PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION} EXPORT_NAME Documentation)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/documentationexport.h
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/documentation COMPONENT Devel
 )
