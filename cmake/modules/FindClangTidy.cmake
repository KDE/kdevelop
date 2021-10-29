# Find the clang-tidy executable
#
# Defines the following variables
#  ClangTidy_EXECUTABLE - path of the Clang-Tidy executable

#=============================================================================
# SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(ClangTidy_EXECUTABLE NAMES clang-tidy)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangTidy DEFAULT_MSG ClangTidy_EXECUTABLE)

mark_as_advanced(ClangTidy_EXECUTABLE)
