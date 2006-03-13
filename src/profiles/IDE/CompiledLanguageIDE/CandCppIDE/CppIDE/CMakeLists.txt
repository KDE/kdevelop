kde4_header()

add_subdirectory(KDECppIDE) 

########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CppIDE FILES profile.config)

kde4_footer()
