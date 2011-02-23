set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( 
    ${QMakeManager_SOURCE_DIR}/parser
    ${QMakeManager_BINARY_DIR}/parser
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(qmakefiletest_SRCS test_qmakefile.cpp
    ../qmakefile.cpp ../qmakeincludefile.cpp ../qmakeprojectfile.cpp
    ../variablereferenceparser.cpp ../qmakemkspecs.cpp )

kde4_add_unit_test(qmakefiletest ${qmakefiletest_SRCS})
target_link_libraries(qmakefiletest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4qmakeparser
    )

set(qmakeprojectfiletest_SRCS test_qmakeprojectfile.cpp
    ../qmakeprojectfile.cpp ../qmakefile.cpp ../qmakeincludefile.cpp ../qmakeprojectfile.cpp
    ../variablereferenceparser.cpp ../qmakemkspecs.cpp )

kde4_add_unit_test(qmakeprojectfiletest ${qmakeprojectfiletest_SRCS})
target_link_libraries(qmakeprojectfiletest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdev4qmakeparser
    )