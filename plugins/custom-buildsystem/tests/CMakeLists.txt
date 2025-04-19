remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

configure_file(testconfig.h.in testconfig.h)

ecm_add_test(test_custombuildsystemplugin.cpp ${custom_LOG_SRCS}
    TEST_NAME test_custombuildsystemplugin
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Project)

# Small sample app loading a dialog with the config-ui and a dummy project
# Useful for testing UI ideas quickly
set( kcm_uitest_SRCS kcmuitestmain.cpp
                     ../custombuildsystemconfigwidget.cpp
                     ../configconstants.cpp
                     ../configwidget.cpp
                     ${custom_LOG_SRCS}
   )
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
ki18n_wrap_ui( kcm_uitest_SRCS
                        ../configwidget.ui
                        ../custombuildsystemconfigwidget.ui
                 )
add_executable( kcm_uitest ${kcm_uitest_SRCS} )
target_link_libraries(kcm_uitest
    KDev::Util
    KF6::Completion
    KF6::ConfigCore
    KF6::I18n
    KF6::KIOWidgets
)
