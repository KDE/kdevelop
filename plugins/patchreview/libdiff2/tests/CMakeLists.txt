set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(interactiveDiffTest_SRCS
    interactivedifftest.cpp
    ../perforceparser.cpp
    ../cvsdiffparser.cpp
    ../difference.cpp
    ../diffhunk.cpp
    ../diffmodel.cpp
    ../diffmodellist.cpp
    ../diffparser.cpp
    ../komparemodellist.cpp
    ../kompareprocess.cpp
    ../parser.cpp
    ../parserbase.cpp
    ../stringlistpair.cpp
)

add_executable(interactiveDiffTest ${interactiveDiffTest_SRCS})
ecm_mark_as_test(interactiveDiffTest)
target_link_libraries(interactiveDiffTest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
)

set(levenshteinTest_SRCS
    levenshteintest.cpp
    ../difference.cpp
    ../stringlistpair.cpp
)

add_executable(levenshteintest ${levenshteinTest_SRCS})
ecm_mark_as_test(levenshteintest)
target_link_libraries(levenshteintest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)
