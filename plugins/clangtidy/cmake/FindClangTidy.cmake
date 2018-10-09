# Find the clang-tidy executable
#
# Defines the following variables
#  ClangTidy_EXECUTABLE - path of the Clang-Tidy executable

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

find_program(ClangTidy_EXECUTABLE NAMES clang-tidy)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangTidy DEFAULT_MSG ClangTidy_EXECUTABLE)

mark_as_advanced(ClangTidy_EXECUTABLE)
