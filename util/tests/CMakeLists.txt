set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(embeddedfreetreetest_SRCS test_embeddedfreetree.cpp)

add_executable(embeddedfreetreetest ${embeddedfreetreetest_SRCS})
ecm_mark_as_test(embeddedfreetreetest)
target_link_libraries(embeddedfreetreetest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} kdevplatformlanguage kdevplatformtests)

set(kdevvarlengtharray_SRC test_kdevvarlengtharray.cpp)
add_executable(kdevvarlengtharraytest ${kdevvarlengtharray_SRC})
ecm_mark_as_test(kdevvarlengtharraytest)
target_link_libraries(kdevvarlengtharraytest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
