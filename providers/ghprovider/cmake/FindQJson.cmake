# - Try to find the QJson library
# Once done this will define
#
#  QJSON_FOUND - system has the QJson library
#  QJSON_INCLUDE_DIR - the QJson include directory
#  QJSON_LIBRARY - Link this to use the QJson library
#
# Copyright (c) 2010, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (QJSON_INCLUDE_DIR AND QJSON_LIBRARY)

  # in cache already
  set(QJSON_FOUND TRUE)

else (QJSON_INCLUDE_DIR AND QJSON_LIBRARY)
  if (NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_QJSON QJson)
  endif(NOT WIN32)

  find_path(QJSON_INCLUDE_DIR qjson/parser.h
    HINTS
    ${PC_QJSON_INCLUDE_DIRS}
  )

  find_library(QJSON_LIBRARY NAMES qjson
    HINTS
    ${PC_QJSON_LIBRARY_DIRS}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QJson DEFAULT_MSG QJSON_LIBRARY QJSON_INCLUDE_DIR)

  mark_as_advanced(QJSON_INCLUDE_DIR QJSON_LIBRARY)

endif (QJSON_INCLUDE_DIR AND QJSON_LIBRARY)
