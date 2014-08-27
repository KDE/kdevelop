include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

ecm_add_test(test_highlighting.cpp
    LINK_LIBRARIES KF5::TextEditor Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KDev::Language)
