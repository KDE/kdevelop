
add_subdirectory(CIDE) 
add_subdirectory(CppIDE) 


########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE FILES profile.config)

