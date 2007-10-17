set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
)

########### next target ###############

set(expressionparsertest_SRCS test_expressionparser.cpp)


kde4_add_unit_test(cppexpressionparsertest ${expressionparsertest_SRCS})
target_link_libraries(cppexpressionparsertest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp ${KDEVPLATFORM_LANGUAGE_LIBRARY} kdev4cppduchain)


include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/editor
    ${KDEVPLATFORM_INCLUDE_DIR}/language
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/languages/cpp
)

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)


kde4_add_unit_test(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARY})
