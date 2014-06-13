# Running the test only makes sense if the bzr command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(BZR_FOUND NAMES bzr)

if (BZR_FOUND)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    set(bazaartest_SRCS
        bazaartest.cpp
        ../src/diffjob.cpp
        ../src/bzrannotatejob.cpp
        ../src/bazaarplugin.cpp
        ../src/bazaarutils.cpp
        ../src/copyjob.cpp
    )

    kde4_add_unit_test(kdevbazaar-test ${bazaartest_SRCS})
    target_link_libraries(kdevbazaar-test
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VCS_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
    )
endif (BZR_FOUND)
