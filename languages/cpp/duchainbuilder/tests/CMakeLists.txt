
include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/expressionparser
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/duchainbuilder
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
    kdev4cppparser kdev4cpprpp kdev4cppduchainbuilder kdev4cppexpressionparser ${KDEVPLATFORM_LANGUAGE_LIBRARY})

