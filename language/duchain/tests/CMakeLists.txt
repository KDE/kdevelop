include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(test_duchain_SRCS test_duchain.cpp)

add_executable(test_duchain ${test_duchain_SRCS})
add_test(test_duchain test_duchain)
ecm_mark_as_test(test_duchain)
target_link_libraries(test_duchain KF5::TextEditor
    Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} KDev::Language)

########### next target ###############

kdevplatform_add_test(test_duchainshutdown SOURCES test_duchainshutdown.cpp)
target_link_libraries(test_duchainshutdown
  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

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

add_executable(test_benchhashes test_benchhashes.cpp)
add_test(test_benchhashes test_benchhashes)
ecm_mark_as_test(test_benchhashes)
target_link_libraries(test_benchhashes

  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)
