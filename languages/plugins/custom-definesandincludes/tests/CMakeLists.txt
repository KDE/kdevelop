set( definesandincludestest_SRCS
     definesandincludestest.cpp
     projectsgenerator.cpp
   )

kde4_add_unit_test( definesandincludestest ${definesandincludestest_SRCS} )
target_link_libraries( definesandincludestest
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )

