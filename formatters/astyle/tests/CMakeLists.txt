set(astyletest_SRCS astyletest.cpp
  ../astyle_formatter.cpp
  ../astyle_stringiterator.cpp
  ../lib/ASFormatter.cpp
  ../lib/ASResource.cpp
  ../lib/ASEnhancer.cpp
  ../lib/ASBeautifier.cpp
)

add_executable(astyletest ${astyletest_SRCS})
ecm_mark_as_test(astyletest)
add_test(astyletest astyletest)
target_link_libraries(astyletest
  
  Qt5::Test
  ${KDEVPLATFORM_INTERFACES_LIBRARIES}
  ${KDEVPLATFORM_UTIL_LIBRARIES}
)

