set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
        ${XtestPlugin_SOURCE_DIR}/cppunit
        ${XtestPlugin_SOURCE_DIR}  # for kasserts.h
        ${KDEVPLATFORM_INCLUDE_DIR}
        ${KDEVPLATFORM_INCLUDE_DIR}/veritas
        ${KDEVPLATFORM_INCLUDE_DIR}/veritas/interfaces)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

macro(kdev_qx_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(cppunit-${test_EXEC} ${test_SRCS})
    target_link_libraries(cppunit-${test_EXEC}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VERITAS_LIBRARY}
        kdevcppunit)
endmacro(kdev_qx_add_test)

kdev_qx_add_test(it-cppunitrunnertest cppunitrunnertest.cpp)

add_executable(emptysuite fake_emptysuite.cpp)
target_link_libraries(emptysuite ${QT_QTCORE_LIBRARY} ${CPPUNIT_LIBRARY})

add_executable(sunnysuite fake_sunnysuite.cpp)
target_link_libraries(sunnysuite ${QT_QTCORE_LIBRARY} ${CPPUNIT_LIBRARY})
