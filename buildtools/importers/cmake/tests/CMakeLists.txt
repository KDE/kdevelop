if (KDE4_BUILD_TESTS)

include_directories(
    ${CMAKE_SOURCE_DIR}/buildtools/importers/cmake
    )

set( folderxmltest_SRCS folderxmltest.cpp )
kde4_automoc( ${folderxmltest_SRCS} )
kdevelop_add_test(${folderxmltest_SRCS})
kde4_add_executable(cmake-folderxmltest ${folderxmltest_SRCS})
target_link_libraries(cmake-folderxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( targetxmltest_SRCS targetxmltest.cpp )
kde4_automoc( ${targetxmltest_SRCS} )
kdevelop_add_test(${targetxmltest_SRCS} )
kde4_add_executable(cmake-targetxmltest ${targetxmltest_SRCS})
target_link_libraries(cmake-targetxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( projectxmltest_SRCS projectxmltest.cpp )
kde4_automoc( ${projectxmltest_SRCS} )
kdevelop_add_test( ${projectxmltest_SRCS} )
kde4_add_executable( cmake-projectxmltest ${projectxmltest_SRCS} )
target_link_libraries( cmake-projectxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

add_test(cmake-folderxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-folderxmltest)
add_test(cmake-targetxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-targetxmltest)
add_test(cmake-projectxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-projectxmltest)

endif(KDE4_BUILD_TESTS)
