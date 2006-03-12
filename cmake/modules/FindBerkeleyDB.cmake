# - Try to find Berkeley DB
# Once done this will define
#
#  BERKELEY_DB_FOUND - system has Berkeley DB
#  BERKELEY_DB_INCLUDE_DIR - the Berkeley DB include directory
#  BERKELEY_DB_LIBRARIES - Link these to use Berkeley DB
#  BERKELEY_DB_DEFINITIONS - Compiler switches required for using Berkeley DB
#


FIND_PATH(BERKELEY_DB_INCLUDE_DIR db.h
  /usr/include/
  /usr/include/db4
  /usr/local/include/db4
)

FIND_LIBRARY(BERKELEY_DB_LIBRARIES NAMES db
  PATHS
  /usr/lib
  /usr/local/lib
)

IF(BERKELEY_DB_INCLUDE_DIR AND BERKELEY_DB_LIBRARIES)
   SET(BERKELEY_DB_FOUND TRUE)
ENDIF(BERKELEY_DB_INCLUDE_DIR AND BERKELEY_DB_LIBRARIES)

IF(BERKELEY_DB_FOUND)
  IF(NOT Berkeley_DB_FIND_QUIETLY)
    MESSAGE(STATUS "Found Berkeley DB: ${BERKELEY_DB_LIBRARIES}")
  ENDIF(NOT Berkeley_DB_FIND_QUIETLY)
ELSE(BERKELEY_DB_FOUND)
  IF(Berkeley_DB_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Berkeley DB >= 4.1")
  ENDIF(Berkeley_DB_FIND_REQUIRED)
ENDIF(BERKELEY_DB_FOUND)

# show the BERKELEY_DB_INCLUDE_DIR and BERKELEY_DB_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(BERKELEY_DB_INCLUDE_DIR BERKELEY_DB_LIBRARIES )

