set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(QMAKE_TESTS_PROJECTS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/manual")
set(QMAKE_TESTS_PROJECTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/manual")
file(COPY ${QMAKE_TESTS_PROJECTS_SOURCE_DIR} DESTINATION "${QMAKE_TESTS_PROJECTS_DIR}/..")

get_property(QMAKE_EXECUTABLE_LOCATION TARGET Qt5::qmake PROPERTY LOCATION)
configure_file(qmaketestconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/qmaketestconfig.h)

set(test_qmakefile_SRCS test_qmakefile.cpp
    ../debug.cpp
    ../qmakefile.cpp
    ../qmakeincludefile.cpp
    ../qmakeprojectfile.cpp
    ../qmakefilevisitor.cpp
    ../qmakemkspecs.cpp
    ../qmakeconfig.cpp
    ../variablereferenceparser.cpp)
ecm_add_test(${test_qmakefile_SRCS} TEST_NAME test_qmakefile LINK_LIBRARIES
    Qt5::Test
    KDev::Tests
    kdevqmakeparser
)

set(test_qmakeproject_SRCS test_qmakeproject.cpp
    ../qmakeconfig.cpp
    ../debug.cpp)
ecm_add_test(${test_qmakeproject_SRCS} TEST_NAME test_qmakeproject LINK_LIBRARIES
    Qt5::Test
    KDev::Tests
    KDev::Util
    kdevqmakeparser
)
