include_directories(${CMAKE_CURRENT_BINARY_DIR})

kde4_add_unit_test(kdeprojectsreadertest
    ../kdeprojectsreader.cpp
    ../kdeprojectsmodel.cpp
    kdeprojectsreadertest.cpp)
target_link_libraries(kdeprojectsreadertest ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

