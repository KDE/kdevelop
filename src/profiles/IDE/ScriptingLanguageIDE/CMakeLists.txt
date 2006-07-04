
add_subdirectory(RubyIDE) 
add_subdirectory(PerlIDE) 
add_subdirectory(PHPIDE) 
add_subdirectory(PythonIDE) 
add_subdirectory(ShellIDE) 


########### install files ###############

install( FILES profile.config DESTINATION ${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/ScriptingLanguageIDE )


