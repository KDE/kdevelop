
macro(kdevelop_add_app_templates _templateNames)
    foreach(_templateName ${ARGV})

        get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
        get_filename_component(_baseName ${_tmp_file} NAME_WE)
        if(WIN32)
			set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.zip)
		else(WIN32)
			set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)
		endif(WIN32)

        add_custom_target(${_baseName} ALL DEPENDS ${_template})

        #file(GLOB _files ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}/*)
        #set(_deps)
        #foreach(_file ${_files})
        #    get_filename_component(_fileName ${_file} NAME)
        #    string(COMPARE NOTEQUAL ${_fileName} .kdev_ignore _v1)
        #    string(SUBSTRING ${_fileName} 0 4 _svn)
        #    string(COMPARE NOTEQUAL ${_svn} .svn _v2)
        #    if ( ${_v1} AND ${_v2} )
        #    set(_deps ${_deps} ${_file})
        #    endif ( ${_v1} AND ${_v2} )
        #endforeach(_file)

		if(WIN32)
			add_custom_command(OUTPUT ${_template}
	            COMMAND zip ARGS -r
	                ${_template} .
					-x .svn .kdev_ignore
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
	            #DEPENDS ${_deps}
	        )
		else(WIN32)
			add_custom_command(OUTPUT ${_template}
	            COMMAND tar ARGS -c -C ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
	                --exclude .kdev_ignore --exclude .svn
	                -j -f ${_template} .
	            #DEPENDS ${_deps}
	        )
		endif(WIN32)
        

        #install( FILES ${_templateName}/${_templateName}.kdevtemplate
        #    DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard/templates )
        #install( FILES ${_templateName}/${_templateName}.png
        #    DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard )
        install( FILES ${_template} DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard/templates )
        macro_additional_clean_files(${_template})

    endforeach(_templateName)
endmacro(kdevelop_add_app_templates)

