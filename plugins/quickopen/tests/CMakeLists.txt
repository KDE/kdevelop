set(quickopentestbase_SRCS
    quickopentestbase.cpp
    ../projectfilequickopen.cpp
)

kde4_add_library(quickopentestbase ${quickopentestbase_SRCS})
target_link_libraries(quickopentestbase
    quickopentestbase
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevplatformlanguage
)

set(quickopentest_SRCS
    quickopentest.cpp
    ${quickopentestbase_SRCS}
)

kde4_add_unit_test(quickopentest ${quickopentest_SRCS})
target_link_libraries(quickopentest
    quickopentestbase
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevplatformlanguage
)

# darn, cannot set per-target so useless :(
# set(CMAKE_BUILD_TYPE Release)

set(quickopenbench_SRCS
    quickopenbench.cpp
    ${quickopentestbase_SRCS}
)
kde4_add_unit_test(quickopenbench ${quickopenbench_SRCS})
target_link_libraries(quickopenbench
    quickopentestbase
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevplatformlanguage
)
set_tests_properties(quickopenbench PROPERTIES TIMEOUT 240)
