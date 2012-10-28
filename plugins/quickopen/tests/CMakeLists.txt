set(quickopentest_SRCS
    quickopentest.cpp
)

kde4_add_unit_test(quickopentest ${quickopentest_SRCS})
target_link_libraries(quickopentest
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevplatformlanguage
)
