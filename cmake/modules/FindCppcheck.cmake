# Find the cppcheck executable
#
# Defines the following variables
#  Cppcheck_EXECUTABLE - path of the Cppcheck executable

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

find_program(Cppcheck_EXECUTABLE NAMES cppcheck)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cppcheck DEFAULT_MSG Cppcheck_EXECUTABLE)

mark_as_advanced(Cppcheck_EXECUTABLE)
