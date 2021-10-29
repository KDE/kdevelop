# Find the heaptrack executable
#
# Defines the following variables
#  heaptrack_EXECUTABLE - path of the heaptrack executable

#=============================================================================
# SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(heaptrack_EXECUTABLE NAMES heaptrack)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(heaptrack DEFAULT_MSG heaptrack_EXECUTABLE)

mark_as_advanced(heaptrack_EXECUTABLE)
