set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(tst_itemrepository_SRCS tst_itemrepository.cpp)

kde4_add_unit_test(tst_itemrepository ${tst_itemrepository_SRCS})
target_link_libraries(tst_itemrepository
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

########### next target ###############

set(tst_indexedstring_SRCS tst_indexedstring.cpp)

kde4_add_unit_test(tst_indexedstring ${tst_indexedstring_SRCS})
target_link_libraries(tst_indexedstring
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

########### next target ###############

set(tst_identifier_SRCS tst_identifier.cpp)

kde4_add_unit_test(tst_identifier ${tst_identifier_SRCS})
target_link_libraries(tst_identifier
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)
