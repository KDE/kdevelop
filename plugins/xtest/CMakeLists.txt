project(XtestPlugin)

find_package(CppUnit)
find_package(Check)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qtest)
add_subdirectory(cppunit)
add_subdirectory(check)
