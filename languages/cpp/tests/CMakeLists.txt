set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/languages/cpp
)

########### next target ###############

add_definitions(-DTEST_COMPLETION)

set(cppcodecompletiontest_SRCS test_cppcodecompletion.cpp ../codecompletioncontext.cpp ../stringhelpers.cpp)


kde4_add_unit_test(cppcodecompletiontest ${cppcodecompletiontest_SRCS})
target_link_libraries(cppcodecompletiontest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARY})

