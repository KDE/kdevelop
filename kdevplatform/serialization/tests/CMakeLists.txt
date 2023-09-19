include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

add_library(itemrepositorytestbase STATIC itemrepositorytestbase.cpp)
target_link_libraries(itemrepositorytestbase PUBLIC
    Qt::Test KDev::Serialization KDev::Tests)

if(BUILD_BENCHMARKS)
    ecm_add_test(bench_itemrepository.cpp LINK_LIBRARIES itemrepositorytestbase)
    ecm_add_test(bench_indexedstring.cpp LINK_LIBRARIES itemrepositorytestbase)
    set_tests_properties(bench_itemrepository PROPERTIES TIMEOUT 30)
    set_tests_properties(bench_indexedstring PROPERTIES TIMEOUT 30)
endif()
ecm_add_test(test_itemrepository.cpp LINK_LIBRARIES itemrepositorytestbase)
ecm_add_test(test_itemrepositoryregistry_automatic.cpp
    LINK_LIBRARIES Qt::Test KDev::Serialization KDev::Tests
)
ecm_add_test(test_itemrepositoryregistry_deferred.cpp
    LINK_LIBRARIES Qt::Test KDev::Serialization KDev::Tests
)
ecm_add_test(test_indexedstring.cpp LINK_LIBRARIES itemrepositorytestbase)
ecm_add_test(test_indexedstringview.cpp LINK_LIBRARIES itemrepositorytestbase)
