
add_subdirectory(weaver) 
add_subdirectory(interfaces) 
add_subdirectory(util) 
add_subdirectory(widgets) 
add_subdirectory(debugger) 
add_subdirectory(projectmanager) 

install( FILES kdevexport.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/)

