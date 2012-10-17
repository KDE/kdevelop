kde4_add_unit_test(glslparsertest parsertest.cpp)
target_link_libraries(glslparsertest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBRARY} glslparser)
