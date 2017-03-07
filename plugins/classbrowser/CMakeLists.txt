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
qt5_add_resources(kdevclassbrowser_PART_SRCS kdevclassbrowser.qrc)
kdevplatform_add_plugin(kdevclassbrowser JSON kdevclassbrowser.json SOURCES ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser KF5::TextEditor KDev::Util KDev::Language KDev::Interfaces)
