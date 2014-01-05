
#add_executable(xmltokross main.cpp cppxmlparse.cpp)
#target_link_libraries(xmltokross ${QT_QTCORE_LIBRARY})

#add_executable(xmltokrossimpl abstractclasses.cpp cppxmlparse.cpp)
#target_link_libraries(xmltokrossimpl ${QT_QTCORE_LIBRARY})

add_executable(duchaintokross mainduchain.cpp duchainextractor.cpp dummybsm.cpp duchainreader.cpp project.cpp)
target_link_libraries(duchaintokross ${QT_QTCORE_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDE4_KROSSCORE_LIBS}
    kdevplatforminterfaces kdevplatformproject kdevplatformlanguage sublime kdevplatformutil kdevplatformoutputview kdevplatformshell)

