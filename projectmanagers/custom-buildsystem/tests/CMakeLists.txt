set( custombuildsystemplugintest_SRCS
     custombuildsystemplugintest.cpp
     ../debugarea.cpp
   )

add_definitions( -DPROJECTS_SOURCE_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/projects\\"" )

kde4_add_unit_test( custombuildsystemplugintest ${custombuildsystemplugintest_SRCS} )
target_link_libraries( custombuildsystemplugintest
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )


# Small sample app loading a dialog with the config-ui and a dummy project
# Useful for testing UI ideas quickly
set( kcm_uitest_SRCS kcmuitestmain.cpp
                     ../custombuildsystemconfigwidget.cpp
                     ../configwidget.cpp
                     ../projectpathswidget.cpp
                     ../includesmodel.cpp
                     ../definesmodel.cpp
                     ../includeswidget.cpp
                     ../defineswidget.cpp
                     ../projectpathsmodel.cpp
                     ../debugarea.cpp
   )
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
kde4_add_ui_files( kcm_uitest_SRCS
                        ../projectpathswidget.ui
                        ../configwidget.ui
                        ../includeswidget.ui
                        ../defineswidget.ui
                        ../custombuildsystemconfigwidget.ui
                 )
kde4_add_executable( kcm_uitest ${kcm_uitest_SRCS} )
target_link_libraries( kcm_uitest ${KDE4_KDEUI_LIBS} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} )
