configure_file(testconfig.h.in testconfig.h)

ecm_add_test(test_custombuildsystemplugin.cpp ../debugarea.cpp
    TEST_NAME test_custombuildsystemplugin
    LINK_LIBRARIES Qt5::Test KF5::KDELibs4Support KDev::Tests KDev::Project)

# Small sample app loading a dialog with the config-ui and a dummy project
# Useful for testing UI ideas quickly
set( kcm_uitest_SRCS kcmuitestmain.cpp
                     ../custombuildsystemconfigwidget.cpp
                     ../configwidget.cpp
                     ../debugarea.cpp
   )
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
qt5_wrap_ui( kcm_uitest_SRCS
                        ../configwidget.ui
                        ../custombuildsystemconfigwidget.ui
                 )
add_executable( kcm_uitest ${kcm_uitest_SRCS} )
target_link_libraries( kcm_uitest KF5::KDELibs4Support KDev::Project KDev::Tests )
