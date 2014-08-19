include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

ecm_add_test(test_itemrepository.cpp
    LINK_LIBRARIES KF5::TextEditor Qt5::Test KDev::Serialization ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

ecm_add_test(test_itemrepositoryregistry_automatic.cpp
    LINK_LIBRARIES Qt5::Test KDev::Serialization ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

ecm_add_test(test_itemrepositoryregistry_deferred.cpp
    LINK_LIBRARIES Qt5::Test KDev::Serialization ${KDEVPLATFORM_TESTS_LIBRARIES})

add_subdirectory(serialization)
