set(test_astyle_SRCS test_astyle.cpp
  ../astyle_formatter.cpp
  ../astyle_stringiterator.cpp
  ../debug.cpp
)

ecm_add_test(${test_astyle_SRCS}
    TEST_NAME test_astyle
    LINK_LIBRARIES astylelib Qt5::Test KDev::Interfaces KDev::Util)

