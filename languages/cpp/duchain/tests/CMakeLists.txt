
include_directories(${CMAKE_SOURCE_DIR}/languages/cpp/parser ${CMAKE_SOURCE_DIR}/languages/cpp/duchain ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/editor)

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)
kde4_automoc(${duchaintest_SRCS})

kde4_add_test(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser kdevcpprpp kdevduchain)
add_test(kdevelop-duchaintest ${EXECUTABLE_OUTPUT_PATH}/duchaintest)


