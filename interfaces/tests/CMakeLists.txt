set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_INCLUDES}
                     ${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/sublime
                     ${CMAKE_SOURCE_DIR}/shell
                     ${CMAKE_SOURCE_DIR}/interfaces
                   )

set( scriptingtest_SRCS scriptingtest.cpp )
kde4_add_unit_test( scriptingtest TESTNAME scriptingtest ${scriptingtest_SRCS} )
target_link_libraries( scriptingtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell )

set( script_documentcontroller_SRCS script_documentcontroller.cpp )
kde4_add_unit_test( script_documentcontroller TESTNAME script_documentcontroller ${script_documentcontroller_SRCS} )
target_link_libraries( script_documentcontroller ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell )

set( script_document_SRCS script_document.cpp )
kde4_add_unit_test( script_document TESTNAME script_document ${script_document_SRCS} )
target_link_libraries( script_document ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell )
