# Find the heaptrack executable
#
# Defines the following variables
#  heaptrack_EXECUTABLE - path of the heaptrack executable

#=============================================================================
# Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_program(heaptrack_EXECUTABLE NAMES heaptrack)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(heaptrack DEFAULT_MSG heaptrack_EXECUTABLE)

mark_as_advanced(heaptrack_EXECUTABLE)
