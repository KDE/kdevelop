set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(patchReviewTest_SRCS
    patchreviewtest.cpp
    ../libdiff2/perforceparser.cpp
    ../libdiff2/cvsdiffparser.cpp
    ../libdiff2/difference.cpp
    ../libdiff2/diffhunk.cpp
    ../libdiff2/diffmodel.cpp
    ../libdiff2/diffmodellist.cpp
    ../libdiff2/diffparser.cpp
    ../libdiff2/komparemodellist.cpp
    ../libdiff2/kompareprocess.cpp
    ../libdiff2/levenshteintable.cpp
    ../libdiff2/parser.cpp
    ../libdiff2/parserbase.cpp
    ../settings/diffsettings.cpp
    ../settings/settingsbase.cpp
)

kde4_add_unit_test(patchreviewtest ${patchReviewTest_SRCS})
target_link_libraries(patchreviewtest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)

