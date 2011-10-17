set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(duchaintest_SRCS test_duchain.cpp)

kde4_add_unit_test(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)

