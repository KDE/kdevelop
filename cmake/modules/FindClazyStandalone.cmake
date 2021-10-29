# Find the clazy-standalone executable
#
# Defines the following variables
#  ClazyStandalone_EXECUTABLE - path of the clazy-standalone executable

#=============================================================================
# SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(ClazyStandalone_EXECUTABLE NAMES clazy-standalone)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClazyStandalone DEFAULT_MSG ClazyStandalone_EXECUTABLE)

mark_as_advanced(ClazyStandalone_EXECUTABLE)
