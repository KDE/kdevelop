#
# KDevelop Platform Macros
#
# The following macros are defined here:
#
#  KDEVPLATFORM_ADD_APP_TEMPLATES( template1 ... templateN )
#    Use this to get packaged template archives for the given templates.
#    Parameters should be the directories containing the templates.
#
#  KDEVPLATFORM_INSTALL_KROSSPLUGIN( desktopfile subdir )
#    Use this macro to install a kross plugin contained within subdir and
#    the desktopfile for loading it
#
# Copyright 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.

# package and install the given directories as application templates
macro(kdevplatform_add_app_templates _templateNames)
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
        foreach(_file ${_files})
            get_filename_component(_fileName ${_file} NAME)
            string(COMPARE NOTEQUAL ${_fileName} .kdev_ignore _v1)
            string(REGEX MATCH "\\.svn" _v2 ${_fileName} )
            if(WIN32)
                string(REGEX MATCH "_svn" _v3 ${_fileName} )
            else(WIN32)
                set(_v3 FALSE)
            endif(WIN32)
            if ( _v1 AND NOT _v2 AND NOT _v3 )
                set(_deps ${_deps} ${_file})
            endif ( _v1 AND NOT _v2 AND NOT _v3 )
        endforeach(_file)

        add_custom_target(${_baseName} ALL DEPENDS ${_template})

        if(WIN32)
            add_custom_command(OUTPUT ${_template}
                COMMAND zip ARGS -r -x .svn _svn .kdev_ignore ${_template} .
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
endmacro(kdevplatform_add_app_templates _templateNames)

# This needs to be reworked once we really support kross plugins
#macro(kdevplatform_install_kross_plugin desktopfile subdir)
#    install( FILES ${desktopfile} DESTINATION ${SERVICES_INSTALL_DIR} )
#    install( DIRECTORY ${subdir} DESTINATION ${DATA_INSTALL_DIR}/kdevkrossplugins )
#endmacro(kdevplatform_install_kross_plugin)

