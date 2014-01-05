set(quickopentestbase_SRCS
    quickopentestbase.cpp
    ../projectfilequickopen.cpp
)

add_library(quickopentestbase ${quickopentestbase_SRCS})
target_link_libraries(quickopentestbase
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

add_executable(quickopentest ${quickopentest_SRCS})
ecm_mark_as_test(quickopentest)
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
add_executable(quickopenbench ${quickopenbench_SRCS})
ecm_mark_as_test(quickopenbench)
target_link_libraries(quickopenbench
    quickopentestbase
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevplatformlanguage
)
# TODO KF5: Port?
#set_tests_properties(quickopenbench PROPERTIES TIMEOUT 240)
