set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_BINARY_DIR}/languages/cpp
)

########### next target ###############

set(expressionparsertest_SRCS test_expressionparser.cpp testhelper.cpp)


kde4_add_unit_test(cppexpressionparsertest ${expressionparsertest_SRCS})
target_link_libraries(cppexpressionparsertest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp ${KDEVPLATFORM_LANGUAGE_LIBRARIES} kdev4cppduchain ${KDEVPLATFORM_TESTS_LIBRARIES}  )


include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_SOURCE_DIR}/languages/cpp
)

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp test_duchain_cpp2011.cpp testhelper.cpp)


kde4_add_unit_test(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(sourcemanipulationtest_SRCS test_sourcemanipulation.cpp testhelper.cpp)


kde4_add_unit_test(sourcemanipulationtest ${sourcemanipulationtest_SRCS})
target_link_libraries(sourcemanipulationtest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

########## next target ################

kde4_add_unit_test(codeanalysistest codeanalysistest.cpp testhelper.cpp)
target_link_libraries(codeanalysistest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

########## next target ################

kde4_add_unit_test(environmenttest testenvironment.cpp testhelper.cpp)
target_link_libraries(environmenttest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY} ${QT_QTTEST_LIBRARY}
    kdev4cppparser kdev4cpprpp kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})