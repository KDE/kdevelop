kde4_header()

add_subdirectory(RubyIDE) 
add_subdirectory(PerlIDE) 
add_subdirectory(PHPIDE) 
add_subdirectory(PythonIDE) 
add_subdirectory(ShellIDE) 


########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/ScriptingLanguageIDE FILES profile.config)


kde4_footer()
