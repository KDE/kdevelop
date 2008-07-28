
find_program(CPPTOXML cpptoxml)

# if(CPPTOXML)
    #we create the xmltokross executable
    add_executable(xmltokross main.cpp)
    target_link_libraries(xmltokross ${QT_QTCORE_LIBRARY})
    
# endif(CPPTOXML)
