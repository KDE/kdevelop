
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

install( FILES ${_templateName}/${_templateName}.kdevtemplate DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard/templates )
install( FILES ${_templateName}/${_templateName}.png DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard )
install( FILES ${_template} DESTINATION ${DATA_INSTALL_DIR}/kdevappwizard )
	macro_additional_clean_files(${_template})

    endforeach(_templateName)
endmacro(kdevelop_add_app_templates)



macro(kdevelop_add_test)

    qt4_get_moc_inc_dirs(_moc_INCS)

    foreach(_source ${ARGN})
        get_filename_component(_abs_file ${_source} ABSOLUTE)
        get_filename_component(_baseName ${_abs_file} NAME_WE)

        file(READ ${_abs_file} _contents)
        string(REGEX MATCHALL "#include +[^ ]+\\.moc[\">]" _match "${_contents}")

        if(_match)
            foreach (_current_MOC_INC ${_match})

                string(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")
                set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
                add_custom_command(
                    OUTPUT ${_moc}
                    COMMAND ${QT_MOC_EXECUTABLE}
                    ARGS ${_moc_INCS} ${CMAKE_CURRENT_SOURCE_DIR}/${_source} -o ${_moc}
                    DEPENDS ${_abs_file}
                )
                MACRO_ADD_FILE_DEPENDENCIES(${_abs_FILE} ${_moc})
                macro_additional_clean_files(${_moc})

            endforeach (_current_MOC_INC)
        endif(_match)
    endforeach(_source)

endmacro(kdevelop_add_test _sources)

