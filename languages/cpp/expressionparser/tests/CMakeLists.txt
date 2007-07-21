
include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
)

########### next target ###############

set(expressionparsertest_SRCS test_expressionparser.cpp)


kde4_add_unit_test(cppexpressionparsertest ${expressionparsertest_SRCS})
target_link_libraries(cppexpressionparsertest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY}
    kdevcppparser kdevcpprpp kdevcppduchainbuilder ${KDEVPLATFORM_LANGUAGE_LIBRARY} kdevcppexpressionparser)

