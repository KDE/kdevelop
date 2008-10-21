set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(embeddedfreetreetest_SRCS test_embeddedfreetree.cpp)

kde4_add_unit_test(embeddedfreetreetest ${embeddedfreetreetest_SRCS})
target_link_libraries(embeddedfreetreetest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_LANGUAGE_LIBRARY})
