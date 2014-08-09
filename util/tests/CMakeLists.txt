set(embeddedfreetreetest_SRCS test_embeddedfreetree.cpp)

add_executable(embeddedfreetreetest ${embeddedfreetreetest_SRCS})
add_test(embeddedfreetreetest embeddedfreetreetest)
ecm_mark_as_test(embeddedfreetreetest)
target_link_libraries(embeddedfreetreetest KF5::TextEditor Qt5::Test kdevplatformlanguage kdevplatformtests)

set(kdevvarlengtharray_SRC test_kdevvarlengtharray.cpp)
add_executable(kdevvarlengtharraytest ${kdevvarlengtharray_SRC})
add_test(kdevvarlengtharraytest kdevvarlengtharraytest)
ecm_mark_as_test(kdevvarlengtharraytest)
target_link_libraries(kdevvarlengtharraytest  Qt5::Test)

kde4_add_unit_test(test_objectlist test_objectlist.cpp)
target_link_libraries(test_objectlist ${QT_QTTEST_LIBRARY} kdevplatformutil)

kde4_add_unit_test(test_stringhandler test_stringhandler.cpp)
target_link_libraries(test_stringhandler ${QT_QTTEST_LIBRARY} kdevplatformutil)

kde4_add_unit_test(pathtest test_path.cpp)
target_link_libraries(pathtest
    Qt5::Test
    KF5::KIOCore
    kdevplatformtests
    kdevplatformutil
)

kde4_add_unit_test(test_foregroundlock test_foregroundlock.cpp)
target_link_libraries(test_foregroundlock ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevplatformutil)
