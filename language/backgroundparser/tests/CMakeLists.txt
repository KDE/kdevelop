set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(backgroundparsertest_SRCS test_backgroundparser.cpp testlanguagesupport.cpp testparsejob.cpp)

kde4_add_unit_test(backgroundparsertest ${backgroundparsertest_SRCS})
target_link_libraries(backgroundparsertest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} kdevplatformlanguage)
