set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(fakeqtest1 fakeqtest1_2cmds.cpp)
target_link_libraries(fakeqtest1 ${KDE4_KDECORE_LIBS})

add_executable(fakeqtest2 fakeqtest2_2cmds_fail.cpp)
target_link_libraries(fakeqtest2 ${KDE4_KDECORE_LIBS})

add_executable(fakeqtest3 fakeqtest3_slow.cpp)
target_link_libraries(fakeqtest3 ${KDE4_KDECORE_LIBS})
