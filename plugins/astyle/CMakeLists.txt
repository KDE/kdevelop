add_definitions(-DTRANSLATION_DOMAIN=\"kdevastyle\")

find_package(LibAStyle 3.1)
set_package_properties(LibAStyle PROPERTIES
    DESCRIPTION "Artistic Style library"
    PURPOSE "External library for the astyle plugin"
    URL "http://astyle.sourceforge.net/"
    TYPE RECOMMENDED)

declare_qt_logging_category(kdevastyle_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_ASTYLE
    CATEGORY_BASENAME "astyle"
)

set(kdevastyle_PART_SRCS
    astyle_plugin.cpp
    astyle_preferences.cpp
    astyle_formatter.cpp
    astyle_stringiterator.cpp
    ${kdevastyle_LOG_SRCS}
)

set(astyle_preferences_UI
    astyle_preferences.ui
)

ki18n_wrap_ui(kdevastyle_PART_SRCS ${astyle_preferences_UI} )
kdevplatform_add_plugin(kdevastyle SOURCES ${kdevastyle_PART_SRCS})
target_link_libraries(kdevastyle
    astylelib

    KF6::Parts
    KF6::KIOWidgets
    KF6::TextEditor

    KDev::Interfaces
    KDev::Util)

if(NOT LibAStyle_FOUND)
    add_subdirectory(3rdparty/libastyle)
endif()
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
