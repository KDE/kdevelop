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

if( NOT KDEVELOP_INCLUDE_DIR )
    find_path( _kdevIncDir kdevelop/make/imakebuilder.h
        PATHS
        ${CMAKE_INSTALL_PREFIX}/include
        ${_KDEVELOP_INCLUDE_DIR}
        ${KDE4_INCLUDE_DIR}
    )
    if(_kdevIncDir)
        set(KDEVELOP_INCLUDE_DIR ${_kdevIncDir}/kdevelop)
        set(KDEVELOP_INCLUDE_DIR ${KDEVELOP_INCLUDE_DIR} CACHE PATH "kdevelop include directory containing the various platform modules")
    endif(_kdevIncDir)
else( NOT KDEVELOP_INCLUDE_DIR )
    set(KDEVELOP_INCLUDE_DIR ${KDEVELOP_INCLUDE_DIR} CACHE PATH "kdevelop include directory containing the various platform modules")
endif( NOT KDEVELOP_INCLUDE_DIR )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDevelop DEFAULT_MSG KDEVELOP_INCLUDE_DIR)
