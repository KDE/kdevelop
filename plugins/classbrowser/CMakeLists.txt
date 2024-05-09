add_definitions(-DTRANSLATION_DOMAIN=\"kdevclassbrowser\")
include_directories(
    ${Boost_INCLUDE_DIRS}
)

# workaround a boost bug in 1.37 and 1.38 that causes link failure when exceptions are disabled
# see https://svn.boost.org/trac/boost/ticket/2947 for details
if( ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_PATCH_VERSION} VERSION_GREATER 1.36.1 )
    kde_enable_exceptions()
endif()

set(kdevclassbrowser_PART_SRCS
    classbrowserplugin.cpp
    classwidget.cpp
    classtree.cpp
)
declare_qt_logging_category(kdevclassbrowser_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_CLASSBROWSER
    CATEGORY_BASENAME "classbrowser"
)
qt_add_resources(kdevclassbrowser_PART_SRCS kdevclassbrowser.qrc)
kdevplatform_add_plugin(kdevclassbrowser SOURCES ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser KF6::TextEditor KDev::Util KDev::Language KDev::Interfaces)
