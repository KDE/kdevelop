# Find the meson executable
#
# Defines the following variables
#  Meson_EXECUTABLE - path of the meson executable
#  Meson_VERSION    - version of the found meson executable

#=============================================================================
# SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(Meson_EXECUTABLE NAMES meson meson.py)

set(Meson_VERSION "")

if(Meson_EXECUTABLE)
    execute_process(
        COMMAND ${Meson_EXECUTABLE} --version
        TIMEOUT 5
        OUTPUT_VARIABLE Meson_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Meson REQUIRED_VARS Meson_EXECUTABLE VERSION_VAR Meson_VERSION)

mark_as_advanced(Meson_EXECUTABLE)
mark_as_advanced(Meson_VERSION)
