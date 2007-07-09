
include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/duchainbuilder
    ${CMAKE_SOURCE_DIR}/languages/cpp
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/editor
    ${KDEVPLATFORM_INCLUDE_DIR}/language
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain
)

########### next target ###############

set(cppcodecompletiontest_SRCS test_cppcodecompletion.cpp ../codecompletioncontext.cpp ../stringhelpers.cpp)


kde4_add_unit_test(cppcodecompletiontest ${cppcodecompletiontest_SRCS})
target_link_libraries(cppcodecompletiontest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY}
    kdevcppparser kdevcpprpp kdevcppduchainbuilder ${KDEVPLATFORM_LANGUAGE_LIBRARY} kdevcppexpressionparser)

