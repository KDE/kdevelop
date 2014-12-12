include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT COMPILER_OPTIMIZATIONS_DISABLED)
    ecm_add_test(bench_itemrepository.cpp LINK_LIBRARIES
        LINK_LIBRARIES Qt5::Test KDev::Serialization KDev::Tests)
    set_tests_properties(bench_itemrepository PROPERTIES TIMEOUT 30)
endif()
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
