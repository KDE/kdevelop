set_package_properties(Qt5WebKitWidgets PROPERTIES
    PURPOSE "QtWebKit, for integrated documentation"
    URL "http://qt-project.org/"
    TYPE REQUIRED)

set(kdevplatformdocumentation_LIB_SRCS
    standarddocumentationview.cpp
    documentationfindwidget.cpp
    documentationview.cpp
)

qt5_wrap_ui(kdevplatformdocumentation_LIB_SRCS documentationfindwidget.ui)
add_library(kdevplatformdocumentation SHARED ${kdevplatformdocumentation_LIB_SRCS})

install(TARGETS kdevplatformdocumentation EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries(kdevplatformdocumentation
        LINK_PUBLIC kdevplatforminterfaces Qt5::WebKitWidgets)

set_target_properties(kdevplatformdocumentation PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

install(FILES
    documentationexport.h
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/documentation COMPONENT Devel
 )
