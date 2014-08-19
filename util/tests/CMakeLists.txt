set(test_embeddedfreetree_SRCS test_embeddedfreetree.cpp)

add_executable(test_embeddedfreetree ${test_embeddedfreetree_SRCS})
add_test(test_embeddedfreetree test_embeddedfreetree)
ecm_mark_as_test(test_embeddedfreetree)
target_link_libraries(test_embeddedfreetree KF5::TextEditor Qt5::Test KDev::Language KDev::Tests)

set(test_kdevvarlengtharray_SRC test_kdevvarlengtharray.cpp)
add_executable(test_kdevvarlengtharray ${test_kdevvarlengtharray_SRC})
add_test(test_kdevvarlengtharray test_kdevvarlengtharray)
ecm_mark_as_test(test_kdevvarlengtharray)
target_link_libraries(test_kdevvarlengtharray  Qt5::Test)

kdevplatform_add_test(test_objectlist SOURCES test_objectlist.cpp)
target_link_libraries(test_objectlist Qt5::Test KDev::Util)

kdevplatform_add_test(test_stringhandler SOURCES test_stringhandler.cpp)
target_link_libraries(test_stringhandler Qt5::Test KDev::Util)

kdevplatform_add_test(test_path SOURCES test_path.cpp)
target_link_libraries(test_path
    Qt5::Test
    KF5::KIOCore
    KDev::Tests
    KDev::Util
)

kdevplatform_add_test(test_foregroundlock SOURCES test_foregroundlock.cpp)
target_link_libraries(test_foregroundlock Qt5::Test KDev::Util)
