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
  /usr/include/samba
  /usr/local/samba/include
)

FIND_LIBRARY(TDB_LIBRARIES NAMES tdb
  PATHS
  /usr/lib/samba
  /usr/local/samba/lib
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TDB DEFAULT_MSG TDB_INCLUDE_DIR TDB_LIBRARIES )

# show the TDB_INCLUDE_DIR and TDB_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(TDB_INCLUDE_DIR TDB_LIBRARIES )

