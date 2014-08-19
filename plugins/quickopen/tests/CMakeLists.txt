set(quickopentestbase_SRCS
    quickopentestbase.cpp
    ../projectfilequickopen.cpp
)

add_library(quickopentestbase STATIC ${quickopentestbase_SRCS})
target_link_libraries(quickopentestbase
    
    Qt5::Test
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    KDev::Language
)

set(test_quickopen_SRCS
    test_quickopen.cpp
    ${quickopentestbase_SRCS}
)

add_executable(test_quickopen ${test_quickopen_SRCS})
add_test(test_quickopen test_quickopen)
ecm_mark_as_test(test_quickopen)
target_link_libraries(test_quickopen
    quickopentestbase
    
    Qt5::Test
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    KDev::Language
)

# darn, cannot set per-target so useless :(
# set(CMAKE_BUILD_TYPE Release)

set(test_quickopenbench_SRCS
    test_quickopenbench.cpp
    ${quickopentestbase_SRCS}
)
add_executable(test_quickopenbench ${test_quickopenbench_SRCS})
add_test(test_quickopenbench test_quickopenbench)
ecm_mark_as_test(test_quickopenbench)
target_link_libraries(test_quickopenbench
    quickopentestbase
    
    Qt5::Test
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    KDev::Language
)
# TODO KF5: Port?
#set_tests_properties(quickopenbench PROPERTIES TIMEOUT 240)
