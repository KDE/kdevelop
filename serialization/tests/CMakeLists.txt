include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(itemrepositorytest_SRCS test_itemrepository.cpp)

add_executable(itemrepositorytest ${itemrepositorytest_SRCS})
add_test(itemrepositorytest itemrepositorytest)
ecm_mark_as_test(itemrepositorytest)
target_link_libraries(itemrepositorytest KF5::TextEditor
    Qt5::Test kdevplatformserialization ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_automatic_test_SRCS test_itemrepositoryregistry_automatic.cpp)
add_executable(itemrepositoryregistry_automatic_test ${itemrepositoryregistry_automatic_test_SRCS})
ecm_mark_as_test(itemrepositoryregistry_automatic_test)
target_link_libraries(itemrepositoryregistry_automatic_test Qt5::Test
    kdevplatformserialization ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_deferred_test_SRCS test_itemrepositoryregistry_deferred.cpp)
add_executable(itemrepositoryregistry_deferred_test ${itemrepositoryregistry_deferred_test_SRCS})
ecm_mark_as_test(itemrepositoryregistry_deferred_test)
target_link_libraries(itemrepositoryregistry_deferred_test Qt5::Test
    kdevplatformserialization ${KDEVPLATFORM_TESTS_LIBRARIES})

add_subdirectory(serialization)