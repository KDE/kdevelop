set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)

kde4_add_unit_test(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)

########### next target ###############

set(itemrepositorytest_SRCS test_itemrepository.cpp)

kde4_add_unit_test(itemrepositorytest ${itemrepositorytest_SRCS})
target_link_libraries(itemrepositorytest ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY}
    ${QT_QTTEST_LIBRARY} kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(benchmarks_SRCS benchmarks.cpp)

kde4_add_unit_test(benchmarks ${benchmarks_SRCS})
target_link_libraries(benchmarks ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBRARY}
    ${QT_QTTEST_LIBRARY} kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_automatic_test_SRCS test_itemrepositoryregistry_automatic.cpp)
kde4_add_unit_test(itemrepositoryregistry_automatic_test ${itemrepositoryregistry_automatic_test_SRCS})
target_link_libraries(itemrepositoryregistry_automatic_test ${QT_QTTEST_LIBRARY}
    kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_deferred_test_SRCS test_itemrepositoryregistry_deferred.cpp)
kde4_add_unit_test(itemrepositoryregistry_deferred_test ${itemrepositoryregistry_deferred_test_SRCS})
target_link_libraries(itemrepositoryregistry_deferred_test ${QT_QTTEST_LIBRARY}
    kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

# TODO: Remove when LTS for g++ < 4.3 has ended.
#       See also: languages/cpp/parser/parser.h in KDevelop
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include(CheckIncludeFileCXX)
    set(CMAKE_REQUIRED_FLAGS "-std=c++0x")
    check_include_file_cxx(unordered_map HAVE_UNORDERED_MAP)
    set(CMAKE_REQUIRED_FLAGS "")

    if(HAVE_UNORDERED_MAP)
      message(STATUS "Enabling c++0x support for unordered map")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif(HAVE_UNORDERED_MAP)
endif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/hashconfig.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/hashconfig.h )

set(benchhashes_SRCS benchhashes.cpp)

kde4_add_unit_test(benchhashes ${benchhashes_SRCS})
target_link_libraries(benchhashes
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
)
