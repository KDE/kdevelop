
add_subdirectory(CandCppIDE) 
add_subdirectory(JavaIDE) 
add_subdirectory(CandCppIDE) 
add_subdirectory(AdaIDE) 
add_subdirectory(FortranIDE) 
add_subdirectory(HaskellIDE) 
add_subdirectory(PascalIDE) 


########### install files ###############

install( FILES profile.config DESTINATION ${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE )


