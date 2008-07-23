set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${CMAKE_SOURCE_DIR}/veritas/coverage
        ${CMAKE_SOURCE_DIR}/veritas/coverage/tests)

macro(kdev_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(veritas-${test_EXEC} ${test_SRCS} )
    target_link_libraries(veritas-${test_EXEC}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        kdevplatformveritas)
endmacro(kdev_add_test)

#kdev_add_test()
