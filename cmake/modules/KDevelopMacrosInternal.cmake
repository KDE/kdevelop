#
# KDevelop Private Macros
#
# The following macros are defined here:
#
#   add_compile_flag_if_supported(<flag> [CXX_ONLY])
#   add_target_compile_flag_if_supported(<target> <INTERFACE|PUBLIC|PRIVATE> <flag>)
#
#=============================================================================
# Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
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
