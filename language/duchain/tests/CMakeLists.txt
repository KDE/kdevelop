include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)

add_executable(duchaintest ${duchaintest_SRCS})
add_test(duchaintest duchaintest)
ecm_mark_as_test(duchaintest)
target_link_libraries(duchaintest KF5::TextEditor
    Qt5::Test ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)

########### next target ###############

add_executable(benchhashes benchhashes.cpp)
add_test(benchhashes benchhashes)
ecm_mark_as_test(benchhashes)
target_link_libraries(benchhashes
  
  Qt5::Test
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
)

########### next target ###############

kdevplatform_add_test(tst_duchainshutdown SOURCES tst_duchainshutdown.cpp)
target_link_libraries(tst_duchainshutdown
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
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
