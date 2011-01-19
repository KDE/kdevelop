
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(standardOutputViewTest_SRCS 
    standardoutputviewtest.cpp 
    ../outputwidget.cpp
    ../standardoutputview.cpp
    ../toolviewdata.cpp
)

kde4_add_unit_test(standardoutputview-test ${standardOutputViewTest_SRCS})
target_link_libraries(standardoutputview-test
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)