set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/shell
    )

set( shelldocumentoperationtest_SRCS shelldocumentoperationtest.cpp )
kde4_add_unit_test(shell-documentoperationtest ${shelldocumentoperationtest_SRCS})
target_link_libraries(shell-documentoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell )
