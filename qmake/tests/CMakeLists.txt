include_directories(${CMAKE_SOURCE_DIR}/buildtools/managers/qmake/parser
                    ${CMAKE_SOURCE_DIR}/buildtools/managers/qmake)

GET_GENERATED_SRCS(parser_GEN_SRCS) 

set(parsetest_SRCS parsetest.cpp ${parser_GEN_SRCS} ${parser_STAT_SRCS})

kde4_add_unit_test(qmake-parsetest ${parsetest_SRCS})
target_link_libraries(qmake-parsetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    )

set(assignmenttest_SRCS assignmenttest.cpp ${parser_GEN_SRCS} ${parser_STAT_SRCS})

kde4_add_unit_test(qmake-assignmenttest ${assignmenttest_SRCS})
target_link_libraries(qmake-assignmenttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    )

set(functionscopetest_SRCS functionscopetest.cpp testhelpers.cpp ${parser_GEN_SRCS} ${parser_STAT_SRCS})

kde4_add_unit_test(qmake-functionscopetest ${functionscopetest_SRCS})
target_link_libraries(qmake-functionscopetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    )

set(scopetest_SRCS scopetest.cpp testhelpers.cpp ${parser_GEN_SRCS} ${parser_STAT_SRCS})

kde4_add_unit_test(qmake-scopetest ${scopetest_SRCS})
target_link_libraries(qmake-scopetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    )
