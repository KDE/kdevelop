# SPDX-FileCopyrightText: 2025 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

#[=======================================================================[.rst:
FindQHexEdit
------------

Try to find the QHexEdit library.

This will define the following variables:

``QHexEdit_FOUND``
    TRUE if (the requested version of) QHexEdit is available
``QHexEdit_VERSION``
    The version of QHexEdit
``QHexEdit_LIBRARIES``
    The libraries of QHexEdit for use with target_link_libraries()
``QHexEdit_INCLUDE_DIRS``
    The include dirs of QHexEdit for use with target_include_directories()

If ``QHexEdit_FOUND`` is TRUE, it will also define the following imported
target:

``QHexEdit``
    The QHexEdit library

Check https://github.com/Simsys/qhexedit2/issues/190 for support by native
CMake config files.
#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_QHexEdit2 qhexedit2.pc QUIET)

find_library(QHexEdit_LIBRARIES
    NAMES qhexedit
    HINTS ${PC_QHexEdit2_LIBRARY_DIRS}
)

find_path(QHexEdit_INCLUDE_DIRS
    NAMES qhexedit.h
    PATH_SUFFIXES qhexedit2
    HINTS ${PC_QHexEdit2_INCLUDE_DIRS}
)

set(QHexEdit_VERSION ${PC_QHexEdit2_VERSION})
if(NOT QHexEdit_VERSION)
    set(_header "${QHexEdit_INCLUDE_DIRS}/qhexedit.h")
    if(EXISTS ${_header})
        file(STRINGS ${_header} _QHexEdit_VERSION REGEX "\\version Version") # take from API dox
        string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+" QHexEdit_VERSION ${_QHexEdit_VERSION})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QHexEdit
    REQUIRED_VARS
        QHexEdit_LIBRARIES
        QHexEdit_INCLUDE_DIRS
    VERSION_VAR
        QHexEdit_VERSION
)

if(QHexEdit_FOUND AND NOT TARGET QHexEdit)
    add_library(QHexEdit UNKNOWN IMPORTED)
    set_target_properties(QHexEdit PROPERTIES
        IMPORTED_LOCATION "${QHexEdit_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${PC_QHexEdit2_CFLAGS}"
        INTERFACE_INCLUDE_DIRECTORIES "${QHexEdit_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(QHexEdit_LIBRARIES QHexEdit_INCLUDE_DIRS QHexEdit_VERSION)

include(FeatureSummary)
set_package_properties(QHexEdit PROPERTIES
    DESCRIPTION "Hex editor widget for Qt"
    URL "https://github.com/Simsys/qhexedit2"
)
