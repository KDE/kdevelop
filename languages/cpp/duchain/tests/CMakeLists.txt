
if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/languages/cpp/parser ${CMAKE_SOURCE_DIR}/languages/cpp/duchain ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/editor)

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)
kde4_automoc(${duchaintest_SRCS})

kdevelop_add_test(${duchaintest_SRCS})
kde4_add_executable(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser kdevcpprpp ktexteditor kdevcpplanguage)
add_test(kdevelop-duchaintest ${EXECUTABLE_OUTPUT_PATH}/duchaintest)

endif(KDE4_BUILD_TESTS)

