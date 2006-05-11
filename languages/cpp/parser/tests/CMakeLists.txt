kde4_header()

if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/languages/cpp/parser )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)


########### next target ###############

set(parsertest_SRCS test_parser.cpp)

kdevelop_add_test(${parsertest_SRCS})
kde4_add_executable(parsertest ${parsertest_SRCS})
target_link_libraries(parsertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser)
add_test(kdevelop-parsertest ${EXECUTABLE_OUTPUT_PATH}/parsertest)


########### next target ###############

set(pooltest_SRCS test_pool.cpp)

kdevelop_add_test(${pooltest_SRCS})
kde4_add_executable(pooltest ${pooltest_SRCS})
target_link_libraries(pooltest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser)
add_test(kdevelop-memorypooltest ${EXECUTABLE_OUTPUT_PATH}/pooltest)


endif(KDE4_BUILD_TESTS)

kde4_footer()
