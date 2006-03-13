kde4_header()

add_subdirectory(CandCppIDE) 
add_subdirectory(JavaIDE) 
add_subdirectory(CandCppIDE) 
add_subdirectory(AdaIDE) 
add_subdirectory(FortranIDE) 
add_subdirectory(HaskellIDE) 
add_subdirectory(PascalIDE) 


########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE FILES profile.config)


kde4_footer()
