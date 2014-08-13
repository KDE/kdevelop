set(embeddedfreetreetest_SRCS test_embeddedfreetree.cpp)

add_executable(embeddedfreetreetest ${embeddedfreetreetest_SRCS})
add_test(embeddedfreetreetest embeddedfreetreetest)
ecm_mark_as_test(embeddedfreetreetest)
target_link_libraries(embeddedfreetreetest KF5::TextEditor Qt5::Test KDev::Language KDev::Tests)

set(kdevvarlengtharray_SRC test_kdevvarlengtharray.cpp)
add_executable(kdevvarlengtharraytest ${kdevvarlengtharray_SRC})
add_test(kdevvarlengtharraytest kdevvarlengtharraytest)
ecm_mark_as_test(kdevvarlengtharraytest)
target_link_libraries(kdevvarlengtharraytest  Qt5::Test)

kdevplatform_add_test(test_objectlist SOURCES test_objectlist.cpp)
target_link_libraries(test_objectlist Qt5::Test KDev::Util)

kdevplatform_add_test(test_stringhandler SOURCES test_stringhandler.cpp)
target_link_libraries(test_stringhandler Qt5::Test KDev::Util)

kdevplatform_add_test(pathtest SOURCES test_path.cpp)
target_link_libraries(pathtest
    Qt5::Test
    KF5::KIOCore
    KDev::Tests
    KDev::Util
)

kdevplatform_add_test(test_foregroundlock SOURCES test_foregroundlock.cpp)
target_link_libraries(test_foregroundlock Qt5::Test KDev::Util)
