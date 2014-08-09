########### next target ###############

set(tst_itemrepository_SRCS tst_itemrepository.cpp)

add_executable(tst_itemrepository ${tst_itemrepository_SRCS})
ecm_mark_as_test(tst_itemrepository)
target_link_libraries(tst_itemrepository

  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  kdevplatformserialization
)

########### next target ###############

set(tst_indexedstring_SRCS tst_indexedstring.cpp)

add_executable(tst_indexedstring ${tst_indexedstring_SRCS})
ecm_mark_as_test(tst_indexedstring)
target_link_libraries(tst_indexedstring

  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  kdevplatformserialization
)

