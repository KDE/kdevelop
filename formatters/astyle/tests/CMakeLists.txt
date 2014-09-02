set(test_astyle_SRCS test_astyle.cpp
  ../astyle_formatter.cpp
  ../astyle_stringiterator.cpp
  ../lib/ASFormatter.cpp
  ../lib/ASResource.cpp
  ../lib/ASEnhancer.cpp
  ../lib/ASBeautifier.cpp
)

ecm_add_test(${test_astyle_SRCS}
    TEST_NAME test_astyle
    LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES})

