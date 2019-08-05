# Find the meson executable
#
# Defines the following variables
#  Meson_EXECUTABLE - path of the meson executable
#  Meson_VERSION    - version of the found meson executable

#=============================================================================
# Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
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
