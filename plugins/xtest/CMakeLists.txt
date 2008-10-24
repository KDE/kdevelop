project(XtestPlugin)

find_package(CppUnit)


add_subdirectory(config)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qtest)
if (CPPUNIT_FOUND)
    add_subdirectory(cppunit)
endif (CPPUNIT_FOUND)
if (UNIX) # Check is unix only
    find_package(Check)
    if (CHECK_FOUND)
        add_subdirectory(check)
    endif (CHECK_FOUND)
endif (UNIX)
