set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(..)

macro(kdev_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(gcov-${test_EXEC} ${test_SRCS} )
    target_link_libraries(gcov-${test_EXEC}
        kdevveritascoverage
        ${KDEVPLATFORM_SUBLIME_LIBRARY}
        ${KDEVPLATFORM_INTERFACES_LIBRARY}
        ${KDE4_KTEXTEDITOR_LIBRARY}
        ${KDE4_KPARTS_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS})
endmacro(kdev_add_test)


kdev_add_test(unit-annotationmanager annotationmanagertest.cpp)
kdev_add_test(unit-lcovinfoparser lcovinfoparsertest.cpp)
kdev_add_test(unit-coveredfile coveredfiletest.cpp)
#kdev_add_test(ut-lcovjob lcovjobtest.cpp)
kdev_add_test(unit-reportmodel reportmodeltest.cpp)
kdev_add_test(unit-reportitems reportitemstest.cpp)
kdev_add_test(unit-reportfileitem reportfileitemtest.cpp)
if(NOT MSVC)
kdev_add_test(unit-reportwidget reportwidgettest.cpp)
endif(NOT MSVC)

macro(coverage_add_mem_test test_EXEC prefix)
    add_test(gcov-mem-${test_EXEC}
        ${CMAKE_CURRENT_SOURCE_DIR}/runMemcheck.py ${CMAKE_CURRENT_BINARY_DIR}/gcov-${prefix}-${test_EXEC} ${CMAKE_BINARY_DIR})
endmacro(coverage_add_mem_test)

coverage_add_mem_test(annotationmanager unit)
coverage_add_mem_test(lcovinfoparser unit)
coverage_add_mem_test(coveredfile unit)
coverage_add_mem_test(reportmodel unit)
coverage_add_mem_test(reportitems unit)
coverage_add_mem_test(reportfileitem unit)
coverage_add_mem_test(reportwidget unit)

