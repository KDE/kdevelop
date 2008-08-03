set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro(kdev_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(veritas-${test_EXEC} ${test_SRCS} )
    target_link_libraries(veritas-${test_EXEC}
        kdevveritascoverage
        sublime
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS})
endmacro(kdev_add_test)


kdev_add_test(ut-annotationmanager annotationmanagertest.cpp)
kdev_add_test(ut-lcovinfoparser lcovinfoparsertest.cpp)
kdev_add_test(ut-coveredfile coveredfiletest.cpp)
#kdev_add_test(ut-lcovjob lcovjobtest.cpp)
kdev_add_test(ut-reportmodel reportmodeltest.cpp)
kdev_add_test(ut-reportitems reportitemstest.cpp)
kdev_add_test(ut-reportfileitem reportfileitemtest.cpp)
