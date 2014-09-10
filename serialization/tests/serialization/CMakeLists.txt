########### next target ###############

ecm_add_test(tst_itemrepository.cpp LINK_LIBRARIES
  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  KDev::Serialization
)

########### next target ###############

ecm_add_test(tst_indexedstring.cpp LINK_LIBRARIES
  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  KDev::Serialization
)

