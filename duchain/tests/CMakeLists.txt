include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(testcontexts_SRCS testcontexts.cpp)

add_executable(testcontexts ${testcontexts_SRCS})
add_test(testcontexts testcontexts)
target_link_libraries(testcontexts
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljsduchain
)

########### next target ###############

set(testdeclarations_SRCS testdeclarations.cpp)

add_executable(testdeclarations ${testdeclarations_SRCS})
add_test(testdeclarations testdeclarations)
target_link_libraries(testdeclarations
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljsduchain
)
