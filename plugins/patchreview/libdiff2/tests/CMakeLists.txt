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
    Qt5::Test
    
    KF5::Parts
    KF5::KDE4Support
)

set(levenshteinTest_SRCS
    levenshteintest.cpp
    ../difference.cpp
    ../stringlistpair.cpp
)

add_executable(levenshteintest ${levenshteinTest_SRCS})
ecm_mark_as_test(levenshteintest)
target_link_libraries(levenshteintest
    Qt5::Test
    Qt5::Core
    KF5::KDE4Support
)
