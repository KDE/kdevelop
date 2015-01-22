set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

configure_file(qmaketestconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/qmaketestconfig.h)

include_directories( 
    ${QMakeManager_SOURCE_DIR}/parser
    ${QMakeManager_BINARY_DIR}/parser
)

set(qmakefiletest_SRCS test_qmakefile.cpp
    ../debug.cpp
    ../qmakefile.cpp ../qmakeincludefile.cpp ../qmakeprojectfile.cpp ../qmakefilevisitor.cpp
    ../variablereferenceparser.cpp ../qmakemkspecs.cpp
    ../qmakeconfig.cpp )

set(qmakeprojecttest_SRCS test_qmakeproject.cpp
    ../qmakeconfig.cpp )

add_executable(qmakefiletest ${qmakefiletest_SRCS})
add_test(qmakefiletest qmakefiletest)
ecm_mark_as_test(qmakefiletest)
add_executable(qmakeprojecttest ${qmakeprojecttest_SRCS})
add_test(qmakeprojecttest qmakeprojecttest)
ecm_mark_as_test(qmakeprojecttest)

target_link_libraries(qmakefiletest
    Qt5::Test
    KF5::KDELibs4Support
    KDev::Tests
    kdevqmakeparser
    )

target_link_libraries(qmakeprojecttest
    Qt5::Test
    KF5::KDELibs4Support
    KDev::Tests
    KDev::Util
    kdevqmakeparser
    )
