#
# KDevelop Private Macros
#
# The following macros are defined here:
#
#   add_compile_flag_if_supported(<flag> [CXX_ONLY])
#   add_target_compile_flag_if_supported(<target> <INTERFACE|PUBLIC|PRIVATE> <flag>)
#   declare_qt_logging_category(<sourcevarname>
#                               TYPE LIBRARY|PLUGIN|APP [IDENTIFIER <id>] [CATEGORY_BASENAME <name>]
#                               [HEADER <filename>] [DESCRIPTION <desc>])
#   install_qt_logging_categories(TYPE LIBRARY|APP_PLUGIN)
#   kdevelop_add_private_library(<target> SOURCES <source1> [<source2> [...]])
#

#=============================================================================
# SPDX-FileCopyrightText: 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
# SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

# helper method to ensure consistent cache var names
function(_varname_for_compile_flag_check_result _varname _flag )
    string(REGEX REPLACE "[-=]" "_" _varname ${_flag})
    string(TOUPPER ${_varname} _varname)
    set(_varname "KDEV_HAVE${_varname}" PARENT_SCOPE)
endfunction()


# add_compile_flag_if_supported(<flag> [CXX_ONLY])
# Optional argument CXX_ONLY to set the flag only for C++ code.
# 
# Example:
#   add_compile_flag_if_supported(-Wsuggest-override CXX_ONLY)
function(add_compile_flag_if_supported _flag)
    _varname_for_compile_flag_check_result(_varname ${_flag})
    check_cxx_compiler_flag("${_flag}" "${_varname}")

    if (${${_varname}})
        if("${ARGV1}" STREQUAL "CXX_ONLY")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flag}" PARENT_SCOPE)
        else()
            add_compile_options(${_flag})
        endif()
    endif()
endfunction()


# add_target_compile_flag_if_supported(<target> <INTERFACE|PUBLIC|PRIVATE> <flag>)
# 
# Example:
#   add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-suggest-override")
function(add_target_compile_flag_if_supported _target _scope _flag)
    _varname_for_compile_flag_check_result(_varname ${_flag})
    check_cxx_compiler_flag("${_flag}" "${_varname}")

    if (${${_varname}})
        target_compile_options(${_target} ${_scope} "${_flag}")
    endif()
endfunction()


# helper method
# _declare_qt_logging_category(<sourcevarname>
#                              HEADER <filename> IDENTIFIER <id> CATEGORY_NAME <name>
#                              EXPORT <target> DESCRIPTION <desc>)
#
# Example:
#   _declare_qt_logging_category(Foo_LIB_SRCS
#       HEADER debug.h
#       IDENTIFIER FOO
#       CATEGORY_NAME "bar.foo"
#       DESCRIPTION "The foo of bar"
#       EXPORT BarCategories
#   )
macro(_declare_qt_logging_category sources)
    set(options )
    set(oneValueArgs HEADER IDENTIFIER CATEGORY_NAME EXPORT DESCRIPTION)
    set(multiValueArgs)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_HEADER)
        message(FATAL_ERROR "HEADER needs to be defined when calling _declare_qt_logging_category().")
    endif()
    if(NOT DEFINED ARGS_IDENTIFIER)
        message(FATAL_ERROR "IDENTIFIER needs to be defined when calling _declare_qt_logging_category().")
    endif()
    if(NOT DEFINED ARGS_CATEGORY_NAME)
        message(FATAL_ERROR "CATEGORY_NAME needs to be defined when calling _declare_qt_logging_category().")
    endif()
    if(NOT DEFINED ARGS_DESCRIPTION)
        message(FATAL_ERROR "DESCRIPTION needs to be defined when calling _declare_qt_logging_category().")
    endif()

    ecm_qt_declare_logging_category(${sources}
        HEADER ${ARGS_HEADER}
        IDENTIFIER ${ARGS_IDENTIFIER}
        CATEGORY_NAME ${ARGS_CATEGORY_NAME}
    )

    if (NOT IS_ABSOLUTE "${ARGS_HEADER}")
        set(ARGS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_HEADER}")
    endif()
    list(APPEND ${sources} "${ARGS_HEADER}")

    set(_propertyprefix "KDEV_QT_LOGGING_CATEGORY_${ARGS_EXPORT}")
    set_property(GLOBAL APPEND PROPERTY "${_propertyprefix}_CATEGORIES" ${ARGS_CATEGORY_NAME})
    set_property(GLOBAL PROPERTY "${_propertyprefix}_IDENTIFIER_${ARGS_CATEGORY_NAME}" "${ARGS_IDENTIFIER}")
    set_property(GLOBAL PROPERTY "${_propertyprefix}_DESCRIPTION_${ARGS_CATEGORY_NAME}" "${ARGS_DESCRIPTION}")
endmacro()


set(_platformlib_qt_logging_categories_export_name "KDevPlatformCategories")
set(_app_plugin_qt_logging_categories_export_name "KDevelopCategories")


# declare_qt_logging_category(<sourcevarname>
#                             TYPE LIBRARY|PLUGIN|APP [IDENTIFIER <id>] [CATEGORY_BASENAME <name>]
#                             [HEADER <filename>] [DESCRIPTION <desc>])
#
# CATEGORY_BASENAME <name> is unused of TYPE APP.
# IDENTIFIER <id> defaults to upper-case CATEGORY_BASENAME <name>.
# HEADER <filename> defaults to "debug.h"
# DESCRIPTION <desc> defaults to CATEGORY_BASENAME <name>.
#
# Example:
#   declare_qt_logging_category(Foo_LIB_SRCS
#       TYPE PLUGIN
#       HEADER foodebug.h
#       IDENTIFIER FOO
#       CATEGORY_BASENAME "foo"
#   )
macro(declare_qt_logging_category sources)
    set(options )
    set(oneValueArgs HEADER IDENTIFIER CATEGORY_BASENAME DESCRIPTION TYPE)
    set(multiValueArgs)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_TYPE)
        message(FATAL_ERROR "TYPE needs to be defined when calling declare_qt_logging_category().")
    endif()
    if(NOT DEFINED ARGS_IDENTIFIER)
        if(ARGS_TYPE STREQUAL "LIBRARY" OR ARGS_TYPE STREQUAL "PLUGIN")
            string(TOUPPER ${ARGS_CATEGORY_BASENAME} ARGS_IDENTIFIER)
        else()
            message(FATAL_ERROR "IDENTIFIER needs to be defined when calling declare_qt_logging_category().")
        endif()
    endif()
    if(NOT DEFINED ARGS_CATEGORY_BASENAME)
        if(ARGS_TYPE STREQUAL "LIBRARY" OR ARGS_TYPE STREQUAL "PLUGIN")
            message(FATAL_ERROR "CATEGORY_BASENAME needs to be defined when calling declare_qt_logging_category() with TYPE of LIBRARY or PLUGIN.")
        endif()
    endif()
    if(NOT DEFINED ARGS_HEADER)
        set(ARGS_HEADER debug.h)
    endif()
    if(NOT DEFINED ARGS_DESCRIPTION)
        set(ARGS_DESCRIPTION ${ARGS_CATEGORY_BASENAME})
    endif()

    if(ARGS_TYPE STREQUAL "LIBRARY")
        _declare_qt_logging_category(${sources}
            HEADER ${ARGS_HEADER}
            IDENTIFIER ${ARGS_IDENTIFIER}
            CATEGORY_NAME "kdevplatform.${ARGS_CATEGORY_BASENAME}"
            DESCRIPTION "KDevPlatform lib: ${ARGS_DESCRIPTION}"
            EXPORT ${_platformlib_qt_logging_categories_export_name}
        )
    elseif(ARGS_TYPE STREQUAL "PLUGIN")
        _declare_qt_logging_category(${sources}
            HEADER ${ARGS_HEADER}
            IDENTIFIER ${ARGS_IDENTIFIER}
            CATEGORY_NAME "kdevelop.plugins.${ARGS_CATEGORY_BASENAME}"
            DESCRIPTION "KDevelop plugin: ${ARGS_DESCRIPTION}"
            EXPORT ${_app_plugin_qt_logging_categories_export_name}
        )
    elseif(ARGS_TYPE STREQUAL "APP")
        _declare_qt_logging_category(${sources}
            HEADER ${ARGS_HEADER}
            IDENTIFIER ${ARGS_IDENTIFIER}
            CATEGORY_NAME "kdevelop.app"
            DESCRIPTION "KDevelop app"
            EXPORT ${_app_plugin_qt_logging_categories_export_name}
        )
    else()
        message(FATAL_ERROR "Unknown \"${ARGS_TYPE}\" with TYPE when calling declare_qt_logging_category().")
    endif()
endmacro()


# helper method
# _install_qt_logging_categories(EXPORT <target> FILE <name> MACRONAME <macro>)
#
# Needs to be called after the last _declare_qt_logging_category call which uses the same EXPORT name.
#
# Example:
#   _install_qt_logging_categories(
#       EXPORT KDevPlatformCategories
#       FILE kdevplatform.categories
#   )
function(_install_qt_logging_categories)
    set(options )
    set(oneValueArgs FILE EXPORT MACRONAME)
    set(multiValueArgs)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_FILE)
        message(FATAL_ERROR "FILE needs to be defined when calling _install_qt_logging_categories().")
    endif()

    if(NOT DEFINED ARGS_EXPORT)
        message(FATAL_ERROR "EXPORT needs to be defined when calling _install_qt_logging_categories().")
    endif()

    if(NOT DEFINED ARGS_MACRONAME)
        message(FATAL_ERROR "MACRONAME needs to be defined when calling _install_qt_logging_categories().")
    endif()

    set(_propertyprefix "KDEV_QT_LOGGING_CATEGORY_${ARGS_EXPORT}")
    get_property(has_category GLOBAL PROPERTY "${_propertyprefix}_CATEGORIES" SET)

    if (NOT has_category)
        message(FATAL_ERROR "${ARGS_EXPORT} is an unknown qt logging category export name.")
    endif()

    get_property(_categories GLOBAL PROPERTY "${_propertyprefix}_CATEGORIES")
    list(SORT _categories)

    set(_content
"# KDebugSettings data file
# This file was generated by ${ARGS_MACRONAME}(). DO NOT EDIT!

")

    foreach(_category IN LISTS _categories)
        get_property(_description GLOBAL PROPERTY "${_propertyprefix}_DESCRIPTION_${_category}")
        get_property(_identifier GLOBAL PROPERTY "${_propertyprefix}_IDENTIFIER_${_category}")

        # Format:
        # logname<space>description(optional <space> DEFAULT_SEVERITY [DEFAULT_CATEGORY] as WARNING/DEBUG/INFO/CRITICAL) optional IDENTIFIER [...])
        string(APPEND _content "${_category} ${_description} IDENTIFIER [${_identifier}]\n")
    endforeach()

    if (NOT IS_ABSOLUTE ${ARGS_FILE})
        set(ARGS_FILE "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_FILE}")
    endif()
    file(GENERATE
        OUTPUT "${ARGS_FILE}"
        CONTENT "${_content}"
    )

    install(
        FILES "${ARGS_FILE}"
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
    )
endfunction()

# install_qt_logging_categories(TYPE LIBRARY|APP_PLUGIN)
#
# Needs to be called after the last declare_qt_logging_category call for the same TYPE(s).
function(install_qt_logging_categories)
    set(options )
    set(oneValueArgs TYPE)
    set(multiValueArgs)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_TYPE)
        message(FATAL_ERROR "TYPE needs to be defined when calling install_qt_logging_categories().")
    endif()

    if(ARGS_TYPE STREQUAL "LIBRARY")
        _install_qt_logging_categories(
            EXPORT ${_platformlib_qt_logging_categories_export_name}
            FILE kdevplatform.categories
            MACRONAME install_qt_logging_categories
        )
    elseif(ARGS_TYPE STREQUAL "APP_PLUGIN")
        _install_qt_logging_categories(
            EXPORT ${_app_plugin_qt_logging_categories_export_name}
            FILE kdevelop.categories
            MACRONAME install_qt_logging_categories
        )
    else()
        message(FATAL_ERROR "Unknown \"${ARGS_TYPE}\" with TYPE when calling declare_qt_logging_category().")
    endif()
endfunction()

# kdevelop_add_private_library(<target> SOURCES <source1> [<source2> [...]])
function(kdevelop_add_private_library target)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(REPLACE "KDev" "" shortTargetName ${target})
    if (${shortTargetName} STREQUAL ${target})
        message(FATAL_ERROR "Target passed to kdevelop_add_private_library needs to start with \"KDev\", was \"${target}\"")
    endif()

    string(TOLOWER ${shortTargetName} shortTargetNameToLower)

    add_library(${target} SHARED ${ARGS_SOURCES})
    add_library(KDev::${shortTargetName} ALIAS ${target})

    generate_export_header(${target} EXPORT_FILE_NAME ${shortTargetNameToLower}export.h)

    set_target_properties(${target} PROPERTIES
        VERSION ${KDEV_PLUGIN_VERSION}
        SOVERSION ${KDEV_PLUGIN_VERSION}
    )

    install(TARGETS ${target} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
endfunction()
