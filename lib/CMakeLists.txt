kde4_header()

add_subdirectory(weaver) 
add_subdirectory(interfaces) 
add_subdirectory(util) 
add_subdirectory(widgets) 
add_subdirectory(debugger) 
add_subdirectory(projectmanager) 

install_files( /include/kdevelop/ FILES kdevexport.h )

kde4_footer()
