include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

ecm_add_test(bench_itemrepository.cpp LINK_LIBRARIES
    LINK_LIBRARIES Qt5::Test KDev::Serialization KDev::Tests
)
ecm_add_test(test_itemrepository.cpp
    LINK_LIBRARIES Qt5::Test KDev::Serialization KDev::Tests
)
ecm_add_test(test_itemrepositoryregistry_automatic.cpp
    LINK_LIBRARIES Qt5::Test KDev::Serialization KDev::Tests
)
ecm_add_test(test_itemrepositoryregistry_deferred.cpp
    LINK_LIBRARIES Qt5::Test KDev::Serialization KDev::Tests
)
ecm_add_test(test_indexedstring.cpp LINK_LIBRARIES
    LINK_LIBRARIES Qt5::Test KDev::Serialization KDev::Tests
)
