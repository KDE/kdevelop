include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(serialization)

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)

add_executable(duchaintest ${duchaintest_SRCS})
ecm_mark_as_test(duchaintest)
target_link_libraries(duchaintest KF5::TextEditor
    Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)

########### next target ###############

set(itemrepositorytest_SRCS test_itemrepository.cpp)

add_executable(itemrepositorytest ${itemrepositorytest_SRCS})
ecm_mark_as_test(itemrepositorytest)
target_link_libraries(itemrepositorytest KF5::TextEditor
    Qt5::Test kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_automatic_test_SRCS test_itemrepositoryregistry_automatic.cpp)
add_executable(itemrepositoryregistry_automatic_test ${itemrepositoryregistry_automatic_test_SRCS})
ecm_mark_as_test(itemrepositoryregistry_automatic_test)
target_link_libraries(itemrepositoryregistry_automatic_test ${QT_QTTEST_LIBRARY}
    kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_deferred_test_SRCS test_itemrepositoryregistry_deferred.cpp)
add_executable(itemrepositoryregistry_deferred_test ${itemrepositoryregistry_deferred_test_SRCS})
ecm_mark_as_test(itemrepositoryregistry_deferred_test)
target_link_libraries(itemrepositoryregistry_deferred_test Qt5::Test
    kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

add_executable(benchhashes benchhashes.cpp)
ecm_mark_as_test(benchhashes)
target_link_libraries(benchhashes
  
  Qt5::Test
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
)
