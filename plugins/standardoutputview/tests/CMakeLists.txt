set(standardOutputViewTest_SRCS
    standardoutputviewtest.cpp
    ../outputwidget.cpp
    ../toolviewdata.cpp
)

add_executable(standardoutputview-test ${standardOutputViewTest_SRCS})
ecm_mark_as_test(standardoutputview-test)
target_link_libraries(standardoutputview-test
    Qt5::Test
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)
