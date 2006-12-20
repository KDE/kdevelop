# - Try to find Samba's TDB (Trivial Database)
# Once done this will define
#
#  TDB_FOUND - system has TDB
#  TDB_INCLUDE_DIR - the TDB include directory
#  TDB_LIBRARIES - Link these to use TDB
#  TDB_DEFINITIONS - Compiler switches required for using TDB
#
# Copyright (c) 2006, Alexander Dymo, <adymo@kdevelop.org>
# Copyright (c) 2006, Jakob Petsovits, <jpetso@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PATH(TDB_INCLUDE_DIR tdb.h
  /usr/include/
  /usr/include/samba
  /usr/local/samba/include
)

FIND_LIBRARY(TDB_LIBRARIES NAMES tdb
  PATHS
  /usr/lib
  /usr/lib/samba
  /usr/local/samba/lib
)

IF(TDB_INCLUDE_DIR AND TDB_LIBRARIES)
   SET(TDB_FOUND TRUE)
ENDIF(TDB_INCLUDE_DIR AND TDB_LIBRARIES)

IF(TDB_FOUND)
  IF(NOT TDB_FIND_QUIETLY)
    MESSAGE(STATUS "Found TDB: ${TDB_LIBRARIES}")
  ENDIF(NOT TDB_FIND_QUIETLY)
ELSE(TDB_FOUND)
  IF(TDB_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find TDB")
  ENDIF(TDB_FIND_REQUIRED)
ENDIF(TDB_FOUND)

# show the TDB_INCLUDE_DIR and TDB_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(TDB_INCLUDE_DIR TDB_LIBRARIES )

