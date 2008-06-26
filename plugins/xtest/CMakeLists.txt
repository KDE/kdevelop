project(XtestPlugin)

find_package(CppUnit)
find_package(Check)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qtest)
if (CPPUNIT_FOUND)
    add_subdirectory(cppunit)
endif (CPPUNIT_FOUND)
if (CHECK_FOUND)
    add_subdirectory(check)
endif (CHECK_FOUND)
