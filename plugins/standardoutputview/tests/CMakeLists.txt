set(test_standardOutputView_SRCS
    test_standardoutputview.cpp
    ../outputwidget.cpp
    ../toolviewdata.cpp
)

add_executable(test_standardoutputview ${test_standardOutputView_SRCS})
add_test(test_standardoutputview test_standardoutputview)
ecm_mark_as_test(test_standardoutputview)
target_link_libraries(test_standardoutputview
    Qt5::Test
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)
