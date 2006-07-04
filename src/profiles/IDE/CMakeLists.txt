
add_subdirectory(CompiledLanguageIDE) 
add_subdirectory(ScriptingLanguageIDE) 
add_subdirectory(DatabaseIDE) 


########### install files ###############

install( FILES profile.config DESTINATION ${DATA_INSTALL_DIR}/kdevelop/profiles/IDE )

