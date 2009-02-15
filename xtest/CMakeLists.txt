project(XtestPlugin)

add_subdirectory(config)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qtest)
