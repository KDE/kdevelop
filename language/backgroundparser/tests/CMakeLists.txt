set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(backgroundparsertest_SRCS test_backgroundparser.cpp testlanguagesupport.cpp testparsejob.cpp)

add_executable(backgroundparsertest ${backgroundparsertest_SRCS})
ecm_mark_as_test(backgroundparsertest)
target_link_libraries(backgroundparsertest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDE4_THREADWEAVER_LIBRARIES} kdevplatformlanguage)
