
include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/duchainbuilder
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/editor
    ${KDEVPLATFORM_INCLUDE_DIR}/language
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain
)

########### next target ###############

set(expressionparsertest_SRCS test_expressionparser.cpp)
kde4_automoc(${expressionparsertest_SRCS})

kde4_add_test(cppexpressionparsertest ${expressionparsertest_SRCS})
target_link_libraries(cppexpressionparsertest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY}
    kdevcppparser kdevcpprpp kdevcppduchainbuilder ${KDEVPLATFORM_LANGUAGE_LIBRARY} kdevcppexpressionparser)

add_test(kdevelop-cppexpressionparsertest ${EXECUTABLE_OUTPUT_PATH}/cppexpressionparsertest)
