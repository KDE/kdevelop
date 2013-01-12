include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(testcontexts_SRCS testcontexts.cpp)

kde4_add_unit_test(testcontexts ${testcontexts_SRCS})
target_link_libraries(testcontexts
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljsduchain
)

########### next target ###############

set(testdeclarations_SRCS testdeclarations.cpp)

kde4_add_unit_test(testdeclarations ${testdeclarations_SRCS})
target_link_libraries(testdeclarations
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljsduchain
)
