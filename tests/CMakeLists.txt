set( custombuildsystemplugintest_SRCS
     custombuildsystemplugintest.cpp
   )

add_definitions( "-DPROJECTS_SOURCE_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/projects\\\"" )

kde4_add_unit_test( custombuildsystemplugintest ${custombuildsystemplugintest_SRCS} )
target_link_libraries( custombuildsystemplugintest
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )
