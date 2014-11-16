add_definitions(-DKDEVPLATFORMINTERFACES_EXPORT=)

set(foregroundlock_SRC test_foregroundlock.cpp ../foregroundlock.cpp)
add_executable(test_foregroundlock ${foregroundlock_SRC})
target_include_directories(test_foregroundlock PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..) # for export header
add_test(test_foregroundlock test_foregroundlock)
ecm_mark_as_test(test_foregroundlock)
target_link_libraries(test_foregroundlock Qt5::Test)

if( RT_LIBRARY )
    target_link_libraries( test_foregroundlock ${RT_LIBRARY} )
endif()

if( Threads_FOUND )
    target_link_libraries( test_foregroundlock ${CMAKE_THREAD_LIBS_INIT} )
endif()
