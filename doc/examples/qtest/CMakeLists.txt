find_package(KDE4 REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_BINARY_DIR})
set(moneytest_SRCS moneytest.cpp)
qt4_automoc(${moneytest_SRCS})
kde4_add_unit_test(moneytest ${moneytest_SRCS})
target_link_libraries(moneytest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS})
