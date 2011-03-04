set(standardOutputViewTest_SRCS
    standardoutputviewtest.cpp
    ../outputwidget.cpp
    ../toolviewdata.cpp
)

kde4_add_unit_test(standardoutputview-test ${standardOutputViewTest_SRCS})
target_link_libraries(standardoutputview-test
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)
