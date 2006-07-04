
add_subdirectory(CIDE) 
add_subdirectory(CppIDE) 


########### install files ###############

install( FILES profile.config DESTINATION ${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE )

