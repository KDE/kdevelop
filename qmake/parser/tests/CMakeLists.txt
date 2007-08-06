include_directories(${QMakeManager_SOURCE_DIR}/parser)

set(parsetest_SRCS parsetest.cpp )

kde4_add_unit_test(qmake-parsetest ${parsetest_SRCS})
target_link_libraries(qmake-parsetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4qmakeparser
    )

set(assignmenttest_SRCS assignmenttest.cpp)

kde4_add_unit_test(qmake-assignmenttest ${assignmenttest_SRCS})
target_link_libraries(qmake-assignmenttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4qmakeparser
    )

set(functionscopetest_SRCS functionscopetest.cpp testhelpers.cpp)

kde4_add_unit_test(qmake-functionscopetest ${functionscopetest_SRCS})
target_link_libraries(qmake-functionscopetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4qmakeparser
    )

set(scopetest_SRCS scopetest.cpp testhelpers.cpp)

kde4_add_unit_test(qmake-scopetest ${scopetest_SRCS})
target_link_libraries(qmake-scopetest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4qmakeparser
    )
