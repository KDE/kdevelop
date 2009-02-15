set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(fakeqtest1 fakeqtest1_2cmds.cpp)
target_link_libraries(fakeqtest1 ${KDE4_KDECORE_LIBS})

add_executable(fakeqtest2 fakeqtest2_2cmds_fail.cpp)
target_link_libraries(fakeqtest2 ${KDE4_KDECORE_LIBS})

add_executable(fakeqtest3 fakeqtest3_slow.cpp)
target_link_libraries(fakeqtest3 ${KDE4_KDECORE_LIBS})

add_executable(nonterminatedOutput nonterminatedOutput.cpp)
target_link_libraries(nonterminatedOutput ${KDE4_KDECORE_LIBS})

macro(add_fake_test test_name)
    qt4_generate_moc( ${test_name}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.moc )
    add_executable( ${test_name} ${test_name}.cpp)
    target_link_libraries( ${test_name} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} )
endmacro(add_fake_test)

add_fake_test(skip)
add_fake_test(segFault)
add_fake_test(multipleFailuresSingleCommand)
add_fake_test(expectedFailure)
add_fake_test(dataTest)
add_fake_test(qassert)
add_fake_test(qassertx)

