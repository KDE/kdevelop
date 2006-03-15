
macro(kdevelop_add_app_templates _templateNames)
    foreach(_templateName ${ARGN})

        get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
        get_filename_component(_baseName ${_tmp_file} NAME_WE)
        set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)

        add_custom_target(${_baseName} ALL DEPENDS ${_template})

	file(GLOB _files ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}/*)
	set(_deps)
	foreach(_file ${_files})
	    get_filename_component(_fileName ${_file} NAME)
	    string(COMPARE NOTEQUAL ${_fileName} ${_templateName}.kdevtemplate _v1)
	    string(COMPARE NOTEQUAL ${_fileName} ${_templateName}.png _v2)
	    string(COMPARE NOTEQUAL ${_fileName} .kdev_ignore _v3)
	    string(COMPARE NOTEQUAL ${_fileName} Makefile.am _v4)
	    string(SUBSTRING ${_fileName} 0 4 _svn)
	    string(COMPARE NOTEQUAL ${_svn} .svn _v5)
	    if ( ${_v1} AND ${_v2} AND ${_v3} AND ${_v4} AND ${_v5} )
		set(_deps ${_deps} ${_file})
	    endif ( ${_v1} AND ${_v2} AND ${_v3} AND ${_v4} AND ${_v5} )
	endforeach(_file)

        add_custom_command(OUTPUT ${_template}
            COMMAND tar ARGS -c -C ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
                --exclude ${_templateName}.kdevtemplate --exclude ${_templateName}.png
                --exclude .kdev_ignore --exclude .svn --exclude Makefile.am
                -j -f ${_template} .
#	    DEPENDS ${_deps}
        )

        install_files(${DATA_INSTALL_DIR}/kdevappwizard/templates FILES ${_templateName}/${_templateName}.kdevtemplate)
        install_files(${DATA_INSTALL_DIR}/kdevappwizard FILES ${_templateName}/${_templateName}.png)
        install_files(${DATA_INSTALL_DIR}/kdevappwizard FILES ${_template})
	macro_additional_clean_files(${_template})

    endforeach(_templateName)
endmacro(kdevelop_add_app_templates)
