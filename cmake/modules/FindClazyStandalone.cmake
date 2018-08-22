# Find the clazy-standalone executable
#
# Defines the following variables
#  ClazyStandalone_EXECUTABLE - path of the clazy-standalone executable

#=============================================================================
# Copyright 2018 Anton Anikin <anton@anikin.xyz>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_program(ClazyStandalone_EXECUTABLE NAMES clazy-standalone)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClazyStandalone DEFAULT_MSG ClazyStandalone_EXECUTABLE)

mark_as_advanced(ClazyStandalone_EXECUTABLE)
