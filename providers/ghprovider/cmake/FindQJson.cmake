# Find QJSON - JSON handling library for Qt
#
# This module defines
#  QJSON_FOUND - whether the qsjon library was found
#  QJSON_LIBRARIES - the qjson library
#  QJSON_INCLUDE_DIR - the include path of the qjson library
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@FreeBSD.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# QJSON v0.7.2+ provides a QJSONConfig.cmake, which should be used if found.
find_package(QJSON QUIET NO_MODULE)

if (QJSON_FOUND)
    set(REQUIRED_LIBS QJSON_CONFIG)
else (QJSON_FOUND)
    find_package(PkgConfig)
    pkg_check_modules(PC_QJSON QJson>=0.5)

    find_library(QJSON_LIBRARIES
        NAMES qjson
        HINTS ${PC_QJSON_LIBDIR} ${PC_QJSON_LIBRARY_DIRS}
    )

    find_path(QJSON_INCLUDE_DIR
        NAMES qjson/parser.h
        HINTS ${PC_QJSON_INCLUDEDIR} ${PC_QJSON_INCLUDE_DIRS}
    )

    set(REQUIRED_LIBS QJSON_LIBRARIES QJSON_INCLUDE_DIR)
endif (QJSON_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QJSON DEFAULT_MSG ${REQUIRED_LIBS})

