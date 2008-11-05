
find_path(CPPUNIT_INCLUDE_DIR
    TestSuite.h PATH_SUFFIXES cppunit )

find_library(CPPUNIT_LIBRARY
    NAMES cppunit-1.12 cppunit )

if (CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)
    set(CPPUNIT_FOUND on)
endif (CPPUNIT_INCLUDE_DIR AND CPPUNIT_LIBRARY)
macro_log_feature(CPPUNIT_FOUND "CppUnit" "A C++ unit testing framework" "http://cppunit.sourceforge.net" FALSE "" "Needed for building the CppUnit test runner")
