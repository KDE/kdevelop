set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( 
    ${QMakeManager_SOURCE_DIR}/parser 
    ${QMakeManager_BINARY_DIR}/parser
)

set(parsetest_SRCS parsetest.cpp )
add_executable(qmakeparsetest ${parsetest_SRCS})
add_test(qmakeparsetest qmakeparsetest)
ecm_mark_as_test(qmakeparsetest)
target_link_libraries(qmakeparsetest
    Qt5::Test
    KF5::KDELibs4Support
    kdev4qmakeparser
)

set(assignmenttest_SRCS assignmenttest.cpp)
add_executable(qmakeassignmenttest ${assignmenttest_SRCS})
add_test(qmakeassignmenttest qmakeassignmenttest)
ecm_mark_as_test(qmakeassignmenttest)
target_link_libraries(qmakeassignmenttest
    Qt5::Test
    KF5::KDELibs4Support
    kdev4qmakeparser
)

set(functionscopetest_SRCS functionscopetest.cpp testhelpers.cpp)
add_executable(qmakefunctionscopetest ${functionscopetest_SRCS})
add_test(qmakefunctionscopetest qmakefunctionscopetest)
ecm_mark_as_test(qmakefunctionscopetest)
target_link_libraries(qmakefunctionscopetest
    Qt5::Test
    KF5::KDELibs4Support
    kdev4qmakeparser
)

set(scopetest_SRCS scopetest.cpp testhelpers.cpp)
add_executable(qmakescopetest ${scopetest_SRCS})
add_test(qmakescopetest qmakescopetest)
ecm_mark_as_test(qmakescopetest)
target_link_libraries(qmakescopetest
    Qt5::Test
    KF5::KDELibs4Support
    kdev4qmakeparser
)

set(lexertest_SRCS lexertest.cpp)
add_executable(qmakelexertest ${lexertest_SRCS})
add_test(qmakelexertest qmakelexertest)
ecm_mark_as_test(qmakelexertest)
target_link_libraries(qmakelexertest
    Qt5::Test
    KF5::KDELibs4Support
    kdev4qmakeparser
)
