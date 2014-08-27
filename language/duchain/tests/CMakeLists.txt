include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

ecm_add_test(test_duchain.cpp
    LINK_LIBRARIES KF5::TextEditor Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KDev::Language)

########### next target ###############

ecm_add_test(test_duchainshutdown.cpp
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES})

########### next target ###############

set(tst_identifier_SRCS tst_identifier.cpp)

add_executable(tst_identifier ${tst_identifier_SRCS})
ecm_mark_as_test(tst_identifier)
target_link_libraries(tst_identifier

    Qt5::Test
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

########### next target ###############

ecm_add_test(test_benchhashes.cpp
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES}  ${KDEVPLATFORM_LANGUAGE_LIBRARIES})
