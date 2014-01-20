
#add_executable(xmltokross main.cpp cppxmlparse.cpp)
#target_link_libraries(xmltokross ${QT_QTCORE_LIBRARY})

#add_executable(xmltokrossimpl abstractclasses.cpp cppxmlparse.cpp)
#target_link_libraries(xmltokrossimpl ${QT_QTCORE_LIBRARY})

add_executable(duchaintokross mainduchain.cpp duchainextractor.cpp dummybsm.cpp duchainreader.cpp project.cpp)
target_link_libraries(duchaintokross KF5::NotifyConfig KF5::TextEditor KF5::ThreadWeaver
    kdevplatforminterfaces kdevplatformproject kdevplatformlanguage sublime kdevplatformutil kdevplatformoutputview kdevplatformshell)

