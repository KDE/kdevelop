set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro(SVN_CREATE_TEST name)
    kde4_add_unit_test(${name} ${ARGN})
    target_link_libraries(${name}
                              ${QT_QTTEST_LIBRARY}
                              ${QT_QTGUI_LIBRARY}
                              ${KDE4_KDECORE_LIBS}
                              kdevplatformtests
                              kdevplatformutil
                              kdevplatformvcs
                         )
endmacro(SVN_CREATE_TEST)

svn_create_test(svnrecursiveadd svnrecursiveadd.cpp)

svn_create_test(svnimport svnimport.cpp)
