#
# KDevelop Platform Macros
#
# The following macros are defined here:
#
#  KDEVPLATFORM_ADD_APP_TEMPLATES( template1 ... templateN )
#    Use this to get packaged template archives for the given app templates.
#    Parameters should be the directories containing the templates.
#
#  KDEVPLATFORM_ADD_FILE_TEMPLATES( template1 ... templateN )
#    Use this to get packaged template archives for the given file templates.
#    Parameters should be the directories containing the templates.
#
#  KDEVPLATFORM_ADD_PLUGIN( <plugin> JSON <jsonfile> SOURCES <src1> [<src2> [...]] [SKIP_INSTALL] )
#    Use this to get create plugins for the KDevPlatform.
#    Parameters should be the json file with plugin metadata and the sources of the plugin.
#    In case the plugin should not be installed, pass SKIP_INSTALL.
#
# SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
#
# SPDX-License-Identifier: BSD-3-Clause

# creates a template archive from the given directory, internal
macro(kdevplatform_create_template_archive _templateName)
    get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
    get_filename_component(_baseName ${_tmp_file} NAME_WE)
    set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)

    file(GLOB _files "${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}/*")
    set(_deps)
    foreach(_file ${_files})
        get_filename_component(_fileName ${_file} NAME)
        string(COMPARE NOTEQUAL ${_fileName} .kdev_ignore _v1)
        string(REGEX MATCH "\\.svn" _v2 ${_fileName} )
        if(WIN32)
            string(REGEX MATCH "_svn" _v3 ${_fileName} )
        else()
            set(_v3 FALSE)
        endif()
        if ( _v1 AND NOT _v2 AND NOT _v3 )
            set(_deps ${_deps} ${_file})
        endif ( _v1 AND NOT _v2 AND NOT _v3 )
    endforeach(_file)
    list(SORT _deps)

    add_custom_target(${_baseName} ALL DEPENDS ${_template})

    if(WIN32)
        add_custom_command(OUTPUT ${_template}
            COMMAND ${CMAKE_COMMAND} -E tar
            ARGS cfvj ${_template} -- ${_deps}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
            DEPENDS ${_deps}
        )
    else()

        if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
            add_custom_command(OUTPUT ${_template}
                COMMAND tar ARGS -c -C ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
                    --exclude .kdev_ignore --exclude .svn --numeric-owner
                    -j -f ${_template} .
                DEPENDS ${_deps}
            )
        else()
            set(extraArgs "")
            if(DEFINED ENV{SOURCE_DATE_EPOCH})
                # We assume there is tar > 1.28 if a reproducible build is wanted.
                set(extraArgs --mtime="@$ENV{SOURCE_DATE_EPOCH}" --sort=name
                    --pax-option=exthdr.name=%d/PaxHeaders/%f,delete=atime,delete=ctime)
            endif()
            add_custom_command(OUTPUT ${_template}
                COMMAND tar ARGS -c -C ${CMAKE_CURRENT_SOURCE_DIR}/${_templateName}
                    --exclude .kdev_ignore --exclude .svn --mode=go=rX,u+rw,a-s
                    --owner=0 --group=0 --numeric-owner -j -f ${_template} ${extraArgs} .
                DEPENDS ${_deps}
            )
        endif()

    endif()


endmacro(kdevplatform_create_template_archive _templateName)

# package and install the given directory as a template archive
macro(kdevplatform_add_template _installDirectory _templateName)
    kdevplatform_create_template_archive(${_templateName})

    get_filename_component(_tmp_file ${_templateName} ABSOLUTE)
    get_filename_component(_baseName ${_tmp_file} NAME_WE)
    set(_template ${CMAKE_CURRENT_BINARY_DIR}/${_baseName}.tar.bz2)

    install( FILES ${_template} DESTINATION ${_installDirectory})
    GET_DIRECTORY_PROPERTY(_tmp_DIR_PROPS ADDITIONAL_MAKE_CLEAN_FILES )
    list(APPEND _tmp_DIR_PROPS ${_template})
    SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${_tmp_DIR_PROPS}")
endmacro(kdevplatform_add_template _installDirectory _templateName)

macro(kdevplatform_add_app_templates _templateNames)
    foreach(_templateName ${ARGV})
        kdevplatform_add_template(${KDE_INSTALL_DATADIR}/kdevappwizard/templates ${_templateName})
    endforeach(_templateName ${ARGV}) 
endmacro(kdevplatform_add_app_templates _templateNames)

macro(kdevplatform_add_file_templates _templateNames)
    foreach(_templateName ${ARGV})
        kdevplatform_add_template(${KDE_INSTALL_DATADIR}/kdevfiletemplates/templates ${_templateName})
    endforeach(_templateName ${ARGV})
endmacro(kdevplatform_add_file_templates _templateNames)

function(kdevplatform_add_plugin plugin)
    set(options SKIP_INSTALL)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KDEV_ADD_PLUGIN "${options}" "" "${multiValueArgs}" ${ARGN})

    list(LENGTH KDEV_ADD_PLUGIN_SOURCES src_count)
    if (NOT ${src_count} GREATER 0)
        message(FATAL_ERROR "kdevplatform_add_plugin() called without passing any source files. Please use the SOURCES parameter.")
    endif()

    add_library(${plugin} MODULE ${KDEV_ADD_PLUGIN_SOURCES})

    if (NOT KDEV_ADD_PLUGIN_SKIP_INSTALL)
        install(TARGETS ${plugin} DESTINATION ${KDE_INSTALL_PLUGINDIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
    endif()
endfunction()
