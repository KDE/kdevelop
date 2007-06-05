include_directories(${CMAKE_SOURCE_DIR}/buildtools/managers/qmake/parser
                    ${CMAKE_SOURCE_DIR}/buildtools/managers/qmake)

set(parsetest_SRCS parsetest.cpp)
kde4_automoc(qmake-parsetest ${parsetest_SRCS})
kde4_add_test(qmake-parsetest ${parsetest_SRCS})
target_link_libraries(qmake-parsetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevqmakeparser
    )

set(assignmenttest_SRCS assignmenttest.cpp)
kde4_automoc(qmake-assignmenttest ${assignmenttest_SRCS})
kde4_add_test(qmake-assignmenttest ${assignmenttest_SRCS})
target_link_libraries(qmake-assignmenttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevqmakeparser
    )

set(functionscopetest_SRCS functionscopetest.cpp testhelpers.cpp)
kde4_automoc(qmake-functionscopetest ${functionscopetest_SRCS})
kde4_add_test(qmake-functionscopetest ${functionscopetest_SRCS})
target_link_libraries(qmake-functionscopetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevqmakeparser
    )

set(scopetest_SRCS scopetest.cpp testhelpers.cpp)
kde4_automoc(qmake-scopetest ${scopetest_SRCS})
kde4_add_test(qmake-scopetest ${scopetest_SRCS})
target_link_libraries(qmake-scopetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevqmakeparser
    )

add_test(qmake-parsetest ${EXECUTABLE_OUTPUT_PATH}/qmake-parsetest)
add_test(qmake-assignmenttest ${EXECUTABLE_OUTPUT_PATH}/qmake-assignmenttest)
add_test(qmake-functionscopetest ${EXECUTABLE_OUTPUT_PATH}/qmake-functionscopetest)
add_test(qmake-scopetest ${EXECUTABLE_OUTPUT_PATH}/qmake-scopetest)
