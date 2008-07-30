
find_program(CPPTOXML cpptoxml)

# if(CPPTOXML)
    add_executable(xmltokross main.cpp)
    target_link_libraries(xmltokross ${QT_QTCORE_LIBRARY})

    add_executable(xmltokrossimpl abstractclasses.cpp)
    target_link_libraries(xmltokrossimpl ${QT_QTCORE_LIBRARY})
 
# endif(CPPTOXML)
