kde4_header()

add_subdirectory(CompiledLanguageIDE) 
add_subdirectory(ScriptingLanguageIDE) 
add_subdirectory(DatabaseIDE) 


########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE FILES profile.config)

kde4_footer()
