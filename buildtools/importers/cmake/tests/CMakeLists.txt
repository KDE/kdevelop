if (KDE4_BUILD_TESTS)

include_directories(
    ${CMAKE_SOURCE_DIR}/buildtools/importers/cmake
    ${CMAKE_SOURCE_DIR}/buildtools/importers/cmake/parser
    )

set( folderxmltest_SRCS folderxmltest.cpp )
kde4_automoc(${folderxmltest_SRCS})
kde4_add_executable(cmake-folderxmltest NOGUI RUN_UNINSTALLED ${folderxmltest_SRCS})
target_link_libraries(cmake-folderxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( targetxmltest_SRCS targetxmltest.cpp )
kde4_automoc(${targetxmltest_SRCS})
kde4_add_executable(cmake-targetxmltest NOGUI RUN_UNINSTALLED ${targetxmltest_SRCS})
target_link_libraries(cmake-targetxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( projectxmltest_SRCS projectxmltest.cpp )
kde4_automoc(${projectxmltest_SRCS})
kde4_add_executable( cmake-projectxmltest NOGUI RUN_UNINSTALLED ${projectxmltest_SRCS} )
target_link_libraries( cmake-projectxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( cmakeparsertest_SRCS cmakeparsertest.cpp )
kde4_automoc( cmake-parsertest ${cmakeparsertest_SRCS} )
kde4_add_executable( cmake-parsertest NOGUI RUN_UNINSTALLED ${cmakeparsertest_SRCS} )
target_link_libraries( cmake-parsertest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( astfactorytest_SRCS astfactorytest.cpp )
kde4_automoc( cmake-astfactorytest ${astfactorytest_SRCS} )
kde4_add_executable( cmake-astfactorytest NOGUI RUN_UNINSTALLED ${astfactorytest_SRCS} )
target_link_libraries( cmake-astfactorytest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( customcommandasttest_SRCS cmakecustomcommandasttest.cpp )
kde4_automoc( cmake-customcommandasttest ${customcommandasttest_SRCS} )
kde4_add_executable( cmake-customcommandasttest NOGUI RUN_UNINSTALLED 
                     ${customcommandasttest_SRCS} )
target_link_libraries( cmake-customcommandasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( customtargetasttest_SRCS cmakecustomtargetasttest.cpp )
kde4_automoc( cmake-customtargetasttest ${customtargetasttest_SRCS} )
kde4_add_executable( cmake-customtargetasttest NOGUI RUN_UNINSTALLED
                     ${customtargetasttest_SRCS} )
target_link_libraries( cmake-customtargetasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

add_test(cmake-folderxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-folderxmltest)
add_test(cmake-targetxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-targetxmltest)
add_test(cmake-projectxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-projectxmltest)
add_test(cmake-parsertest ${EXECUTABLE_OUTPUT_PATH}/cmake-parsertest)
add_test(cmake-astfactorytest ${EXECUTABLE_OUTPUT_PATH}/cmake-astfactorytest)
add_test(cmake-customcommandtest ${EXECUTABLE_OUTPUT_PATH}/cmake-customcommandasttest)
add_test(cmake-customtargetasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-customtargetasttest)
endif(KDE4_BUILD_TESTS)
