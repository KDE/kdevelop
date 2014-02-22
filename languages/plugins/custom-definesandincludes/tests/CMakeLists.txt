set( plugintest_SRCS
     plugintest.cpp
   )

kde4_add_unit_test( plugintest ${plugintest_SRCS} )
target_link_libraries( plugintest
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )

configure_file(testconfig.h.in testconfig.h)
