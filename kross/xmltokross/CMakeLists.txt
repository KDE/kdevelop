
#add_executable(xmltokross main.cpp cppxmlparse.cpp)
#target_link_libraries(xmltokross ${QT_QTCORE_LIBRARY})

#add_executable(xmltokrossimpl abstractclasses.cpp cppxmlparse.cpp)
#target_link_libraries(xmltokrossimpl ${QT_QTCORE_LIBRARY})

set(KDEVCPPDUCHAIN "" CACHE STRING "kdevcpp location")
if(KDEVCPPDUCHAIN)
    message(STATUS "building duchaintokross ${KDEVCPPDUCHAIN}")
    include_directories(${KDEVCPPDUCHAIN})

    kde4_add_executable(duchaintokross mainduchain.cpp duchainextractor.cpp dummybsm.cpp duchainreader.cpp project.cpp)
    target_link_libraries(duchaintokross ${QT_QTCORE_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} kdev4cppduchain kdevplatforminterfaces kdevplatformproject kdevplatformlanguage sublime kdevplatformutil kdevplatformoutputview kdevplatformshell)
else(KDEVCPPDUCHAIN)
    message(STATUS "You need to define the KDEVCPPDUCHAIN variable first in order to compile the wrappers.")
endif(KDEVCPPDUCHAIN)

