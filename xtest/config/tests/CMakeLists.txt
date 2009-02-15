
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ) # for ui_veritasconfig.h

macro(kdev_add_test test_EXEC)
    set(test_SRCS ${ARGN})
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(qtest-${test_EXEC} ${test_SRCS})
    target_link_libraries(qtest-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
		${QT_QTGUI_LIBRARY}
		${KDE4_KDEUI_LIBS}
		${KDE4_KIO_LIBS}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevqtest)
endmacro(kdev_add_test)

kdev_add_test(unit-configwidget configwidgettest.cpp ../configwidget.cpp fakedetailswidget.cpp)
