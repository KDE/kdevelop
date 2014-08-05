set( test_custommake_SRCS
     test_custommake.cpp
     ../makefileresolver.cpp
   )

kde4_add_unit_test( test_custommake ${test_custommake_SRCS} )
target_link_libraries( test_custommake
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )
