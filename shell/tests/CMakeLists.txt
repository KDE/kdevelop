set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/shell
    )

set( shelldocumentoperationtest_SRCS shelldocumentoperationtest.cpp )
kde4_add_unit_test(shell-documentoperationtest ${shelldocumentoperationtest_SRCS})
target_link_libraries(shell-documentoperationtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime )

set( projectcontrollertest_SRCS projectcontrollertest.cpp )
kde4_add_unit_test(shell-projectcontrollertest ${projectcontrollertest_SRCS})
target_link_libraries(shell-projectcontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime )

set( sessioncontrollertest_SRCS sessioncontrollertest.cpp )
kde4_add_unit_test(shell-sessioncontrollertest ${sessioncontrollertest_SRCS})
target_link_libraries(shell-sessioncontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatformshell sublime )

set( plugincontrollertest_SRCS plugincontrollertest.cpp )
kde4_add_unit_test(shell-plugincontrollertest ${plugincontrollertest_SRCS})
target_link_libraries(shell-plugincontrollertest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatforminterfaces sublime )
