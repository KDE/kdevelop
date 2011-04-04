set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions( "-DQMAKE_TESTS_PROJECTS_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/manual\\\"" )

include_directories( 
    ${QMakeManager_SOURCE_DIR}/parser
    ${QMakeManager_BINARY_DIR}/parser
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(qmakefiletest_SRCS test_qmakefile.cpp
    ../qmakefile.cpp ../qmakeincludefile.cpp ../qmakeprojectfile.cpp
    ../variablereferenceparser.cpp ../qmakemkspecs.cpp
    ../qmakeconfig.cpp )

set(qmakeprojecttest_SRCS test_qmakeproject.cpp
    ../qmakeconfig.cpp )

kde4_add_unit_test(qmakefiletest ${qmakefiletest_SRCS})
kde4_add_unit_test(qmakeprojecttest ${qmakeprojecttest_SRCS})

target_link_libraries(qmakefiletest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdev4qmakeparser
    )

target_link_libraries(qmakeprojecttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdev4qmakeparser
    )
