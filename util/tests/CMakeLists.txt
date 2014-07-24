set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(embeddedfreetreetest_SRCS test_embeddedfreetree.cpp)

kde4_add_unit_test(embeddedfreetreetest ${embeddedfreetreetest_SRCS})
target_link_libraries(embeddedfreetreetest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS} ${QT_QTTEST_LIBRARY} kdevplatformlanguage kdevplatformtests)

set(kdevvarlengtharray_SRC test_kdevvarlengtharray.cpp)
kde4_add_unit_test(kdevvarlengtharraytest ${kdevvarlengtharray_SRC})
target_link_libraries(kdevvarlengtharraytest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

kde4_add_unit_test(test_objectlist test_objectlist.cpp)
target_link_libraries(test_objectlist ${QT_QTTEST_LIBRARY} kdevplatformutil)

kde4_add_unit_test(test_stringhandler test_stringhandler.cpp)
target_link_libraries(test_stringhandler ${QT_QTTEST_LIBRARY} kdevplatformutil)

kde4_add_unit_test(pathtest test_path.cpp)
target_link_libraries(pathtest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatformtests
    kdevplatformlanguage
    kdevplatformproject
)
