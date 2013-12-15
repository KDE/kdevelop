set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(benchitemrepository_SRCS benchitemrepository.cpp)

kde4_add_unit_test(benchitemrepository ${benchitemrepository_SRCS})
target_link_libraries(benchitemrepository
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

########### next target ###############

set(benchindexedstring_SRCS benchindexedstring.cpp)

kde4_add_unit_test(benchindexedstring ${benchindexedstring_SRCS})
target_link_libraries(benchindexedstring
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)
