#
# Find the KDevelop include dir
#
# KDEVELOP_INCLUDE_DIR 
# adjust the directories where the kdevelop headers
#
# KDEVELOP_FOUND - set to TRUE if kdevelop was found 
#                      FALSE otherwise
#
# KDEVELOP_INCLUDE_DIR         - include dir of kdevelop, for example /usr/include/kdevelop
#                                    can be set by the user to select different include dirs
#
# Copyright (c) 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(WIN32)
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _progFiles)
    set(_KDEVELOP_INCLUDE_DIR ${KDE4_INCLUDE_DIR} ${_progFiles}/kdevelop/include)
endif(WIN32)

find_path(KDEVELOP_INCLUDE_DIR make/imakebuilder.h
    PATHS
    ${CMAKE_INSTALL_PREFIX}/include
    ${_KDEVELOP_INCLUDE_DIR}
    ${KDE4_INCLUDE_DIR}
    PATH_SUFFIXES
    kdevelop
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDevelop DEFAULT_MSG KDEVELOP_INCLUDE_DIR)

mark_as_advanced(KDEVELOP_INCLUDE_DIR)