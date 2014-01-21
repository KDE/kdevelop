#
# KDevelop Platform Macros
#
# The following macros are defined here:
#
#  KDEVPLATFORM_ADD_APP_TEMPLATES( template1 ... templateN )
#    Use this to get packaged template archives for the given app templates.
#    Parameters should be the directories containing the templates.

#  KDEVPLATFORM_ADD_FILE_TEMPLATES( template1 ... templateN )
#    Use this to get packaged template archives for the given file templates.
#    Parameters should be the directories containing the templates.
#
#  KDEVPLATFORM_INSTALL_KROSSPLUGIN( desktopfile subdir )
#    Use this macro to install a kross plugin contained within subdir and
#    the desktopfile for loading it
#
# Copyright 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.

# creates a template archive from the given directory
macro(kdevplatform_create_template_archive _templateName)
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
            COMMAND zip ARGS -r ${_template} . -x .svn _svn .kdev_ignore
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
endmacro(kdevplatform_create_template_archive _templateName)

# package and install the given directory as a template archive
macro(kdevplatform_add_template _installDirectory _templateName)
    kdevplatform_create_template_archive(${_templateName})

    get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
    get_filename_component(_baseName ${_tmp_file} NAME_WE)
    if(WIN32)
        set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.zip)
    else(WIN32)
        set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)
    endif(WIN32)

    install( FILES ${_template} DESTINATION ${_installDirectory})
    GET_DIRECTORY_PROPERTY(_tmp_DIR_PROPS ADDITIONAL_MAKE_CLEAN_FILES )
    list(APPEND _tmp_DIR_PROPS ${_template})
    SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${_tmp_DIR_PROPS}")
endmacro(kdevplatform_add_template _installDirectory _templateName)

macro(kdevplatform_add_app_templates _templateNames)
    foreach(_templateName ${ARGV})
        kdevplatform_add_template(${DATA_INSTALL_DIR}/kdevappwizard/templates ${_templateName})
    endforeach(_templateName ${ARGV}) 
endmacro(kdevplatform_add_app_templates _templateNames)

macro(kdevplatform_add_file_templates _templateNames)
    foreach(_templateName ${ARGV})
        kdevplatform_add_template(${DATA_INSTALL_DIR}/kdevfiletemplates/templates ${_templateName})
    endforeach(_templateName ${ARGV}) 
endmacro(kdevplatform_add_file_templates _templateNames)

# This needs to be reworked once we really support kross plugins
#macro(kdevplatform_install_kross_plugin desktopfile subdir)
#    install( FILES ${desktopfile} DESTINATION ${SERVICES_INSTALL_DIR} )
#    install( DIRECTORY ${subdir} DESTINATION ${DATA_INSTALL_DIR}/kdevkrossplugins )
#endmacro(kdevplatform_install_kross_plugin)

