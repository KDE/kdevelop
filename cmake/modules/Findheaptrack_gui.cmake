# Find the heaptrack_gui executable
#
# Defines the following variables
#  heaptrack_gui_EXECUTABLE - path of the heaptrack_gui executable

#=============================================================================
# SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(heaptrack_gui_EXECUTABLE NAMES heaptrack_gui)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(heaptrack_gui DEFAULT_MSG heaptrack_gui_EXECUTABLE)

mark_as_advanced(heaptrack_gui_EXECUTABLE)
