set(embeddedfreetreetest_SRCS test_embeddedfreetree.cpp)

add_executable(embeddedfreetreetest ${embeddedfreetreetest_SRCS})
ecm_mark_as_test(embeddedfreetreetest)
target_link_libraries(embeddedfreetreetest KF5::TextEditor Qt5::Test kdevplatformlanguage kdevplatformtests)

set(kdevvarlengtharray_SRC test_kdevvarlengtharray.cpp)
add_executable(kdevvarlengtharraytest ${kdevvarlengtharray_SRC})
ecm_mark_as_test(kdevvarlengtharraytest)
target_link_libraries(kdevvarlengtharraytest  Qt5::Test)
