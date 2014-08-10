set( test_custommake_SRCS
     test_custommake.cpp
     ../makefileresolver.cpp
   )

kdevplatform_add_test( test_custommake SOURCES ${test_custommake_SRCS} )
target_link_libraries( test_custommake
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
    )
