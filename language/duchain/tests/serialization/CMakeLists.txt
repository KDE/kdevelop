set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(tst_itemrepository_SRCS tst_itemrepository.cpp)

add_executable(tst_itemrepository ${tst_itemrepository_SRCS})
ecm_mark_as_test(tst_itemrepository)
target_link_libraries(tst_itemrepository
  
  Qt5::Test
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

########### next target ###############

set(tst_indexedstring_SRCS tst_indexedstring.cpp)

add_executable(tst_indexedstring ${tst_indexedstring_SRCS})
ecm_mark_as_test(tst_indexedstring)
target_link_libraries(tst_indexedstring
  
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
