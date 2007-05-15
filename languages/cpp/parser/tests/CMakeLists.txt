
include_directories(${CMAKE_SOURCE_DIR}/languages/cpp/parser)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)


########### next target ###############

#set(parsertest_SRCS test_parser.cpp)

#kde4_automoc(${parsertest_SRCS})
#kde4_add_test(parsertest ${parsertest_SRCS})
#target_link_libraries(parsertest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser)
#add_test(kdevelop-parsertest ${EXECUTABLE_OUTPUT_PATH}/parsertest)


########### next target ###############

set(pooltest_SRCS test_pool.cpp)

kde4_automoc(${pooltest_SRCS})
kde4_add_test(pooltest ${pooltest_SRCS})
target_link_libraries(pooltest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser)
add_test(kdevelop-memorypooltest ${EXECUTABLE_OUTPUT_PATH}/pooltest)


