
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)


########### next target ###############

set(parsertest_SRCS test_parser.cpp test_parser_cpp2011.cpp)


kde4_add_unit_test(parsertest ${parsertest_SRCS})
target_link_libraries(parsertest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} kdev4cpprpp kdev4cppparser)


########### next target ###############

set(generatortest_SRCS test_generator.cpp)


kde4_add_unit_test(generatortest ${generatortest_SRCS})
target_link_libraries(generatortest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} kdev4cpprpp kdev4cppparser)


########### next target ###############

set(pooltest_SRCS test_pool.cpp)


kde4_add_unit_test(pooltest ${pooltest_SRCS})
target_link_libraries(pooltest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} kdev4cppparser)


