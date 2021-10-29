# Find the cppcheck executable
#
# Defines the following variables
#  Cppcheck_EXECUTABLE - path of the Cppcheck executable

#=============================================================================
# SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(Cppcheck_EXECUTABLE NAMES cppcheck)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cppcheck DEFAULT_MSG Cppcheck_EXECUTABLE)

mark_as_advanced(Cppcheck_EXECUTABLE)
