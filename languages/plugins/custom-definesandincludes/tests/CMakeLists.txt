set( plugintest_SRCS
     plugintest.cpp
     projectsgenerator.cpp
   )

add_executable(plugintest ${plugintest_SRCS})
ecm_mark_as_test(plugintest)
add_test(plugintest plugintest)
target_link_libraries( plugintest
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )

