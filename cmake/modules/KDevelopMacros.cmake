
macro(kdevelop_add_app_templates _templateNames)
    foreach(_templateName ${ARGV})

        get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
        get_filename_component(_baseName ${_tmp_file} NAME_WE)
        if(WIN32)
            set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.zip)
        else(WIN32)
            set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)
        endif(WIN32)


        file(GLOB _files "${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}/*")
        set(_deps)
        message("Found files to check: ${_files}")
        foreach(_file ${_files})
            get_filename_component(_fileName ${_file} NAME)
            message("Checking filename: ${_fileName}")
            string(COMPARE NOTEQUAL ${_fileName} .kdev_ignore _v1)
            string(REGEX MATCH "\\.svn" _v2 ${_fileName} )
            if(WIN32)
                string(REGEX MATCH "_svn" _v3 ${_fileName} )
            else(WIN32)
                set(_v3 FALSE)
            endif(WIN32)
            message("Check result: ${_v1} ${_v2} ${_v3}")
            if ( _v1 AND NOT _v2 AND NOT _v3 )
                set(_deps ${_deps} ${_file})
            endif ( _v1 AND NOT _v2 AND NOT _v3 )
        endforeach(_file)

        message("Using files as deps: ${_deps} ")

        add_custom_target(${_baseName} ALL DEPENDS ${_template})

        if(WIN32)
            add_custom_command(OUTPUT ${_template}
                COMMAND zip ARGS -r
                    ${_template} .
                    -x .svn _svn .kdev_ignore
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
                DEPENDS ${_deps}
            )
        else(WIN32)
            add_custom_command(OUTPUT ${_template}
                COMMAND tar ARGS -c -C ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
                    --exclude .kdev_ignore --exclude .svn
                    -j -f ${_template} .
                DEPENDS ${_deps}
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

