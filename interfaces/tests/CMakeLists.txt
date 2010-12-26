set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DKDEVPLATFORMINTERFACES_EXPORT=)

set(foregroundlock_SRC test_foregroundlock.cpp ../foregroundlock.cpp)
kde4_add_unit_test(foregroundlocktest ${foregroundlock_SRC})
target_link_libraries(foregroundlocktest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

if( RT_LIBRARY )
    target_link_libraries( foregroundlocktest ${RT_LIBRARY} )
endif( RT_LIBRARY )

if( Threads_FOUND )
    target_link_libraries( foregroundlocktest ${CMAKE_THREAD_LIBS_INIT} )
endif( Threads_FOUND )