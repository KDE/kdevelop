add_definitions(-DKDEVPLATFORMINTERFACES_EXPORT=)

set(foregroundlock_SRC test_foregroundlock.cpp ../foregroundlock.cpp)
add_executable(foregroundlocktest ${foregroundlock_SRC})
add_test(foregroundlocktest foregroundlocktest)
ecm_mark_as_test(foregroundlocktest)
target_link_libraries(foregroundlocktest  Qt5::Test KF5::KDE4Support)

if( RT_LIBRARY )
    target_link_libraries( foregroundlocktest ${RT_LIBRARY} )
endif()

if( Threads_FOUND )
    target_link_libraries( foregroundlocktest ${CMAKE_THREAD_LIBS_INIT} )
endif()
