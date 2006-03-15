kde4_header()


set(apptemplate_DIRS
    kapp kpartapp kpartplugin kcmodule kicker kioslave kdevpart kdevpart2 
    chello cpphello gnomeapp kscreensaver 
#    kofficepart chello_gba
    dcopservice 
    konqnavpanel qmakesimple qmakeapp khello khello2 kateplugin kateplugin2 
    qtopiaapp kdedcop wxhello cppsdlhello kdevlang noatunvisual noatunui 
    kbearplugin kbearimportfilter kmod kfileplugin clanlib opieapp opieapplet 
    opieinput opiemenu opienet opietoday kxt gtk2mmapp cppcurseshello 
    win32hello win32gui cmakesimple cmakesimplec cmakelibcpp cmakelibc kscons_kxt kscons_kmdi
)


#macro(kdevelop_add_app_templates _templateNames)
#    foreach(_templateName ${ARGN})
#	
#	get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
#	get_filename_component(_baseName ${_tmp_file} NAME_WE)
#	set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)
#
#	add_custom_target(${_baseName} ALL DEPENDS ${_template})
#
#        add_custom_command(OUTPUT ${_template}
#	    COMMAND tar ARGS -c -C ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
#		--exclude ${_templateName}.kdevtemplate --exclude ${_templateName}.png
#		--exclude .kdev_ignore --exclude .svn --exclude Makefile.am
#		-j -f ${_template} .
#	)
#
#	install_files(${DATA_INSTALL_DIR}/kdevappwizard/templates FILES ${_templateName}/${_templateName}.kdevtemplate)
#	install_files(${DATA_INSTALL_DIR}/kdevappwizard FILES ${_templateName}/${_templateName}.png)
#	install_files(${DATA_INSTALL_DIR}/kdevappwizard FILES ${_template})
#    
#    endforeach(_templateName)
#endmacro(kdevelop_add_app_templates)


kdevelop_add_app_templates(${apptemplate_DIRS})

########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CppIDE/KDECppIDE FILES kdecpp.appwizard)
install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CppIDE FILES cpp.appwizard)
install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CIDE FILES c.appwizard)


kde4_footer()
