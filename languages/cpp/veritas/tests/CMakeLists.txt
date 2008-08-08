set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${KDEVPlATFORM_INCLUDES})

macro(kdev_add_test test_EXEC test_SRCS)
    qt4_automoc( ${test_SRCS} )
    kde4_add_unit_test(veritas-${test_EXEC} ${test_SRCS})
    target_link_libraries(veritas-${test_EXEC}
        veritascpp
        ${QT_QTTEST_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${KDE4_KDECORE_LIBS})
endmacro(kdev_add_test)

kdev_add_test(ut-classskeleton classskeletontest.cpp)
kdev_add_test(ut-stubcreation stubcreationtest.cpp)
kdev_add_test(ut-constructorskeleton constructorskeletontest.cpp)
kdev_add_test(ut-methodskeleton methodskeletontest.cpp)
#kdev_add_test(ut-classwriter classwritertest.cpp)
