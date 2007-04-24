# - Try to find libcommoncpp2
# Once done this will define
#
#  COMMONCPP2_FOUND - system has libcommoncpp2
#  COMMONCPP2_INCLUDE_DIR - the commoncpp2 include directory
#  COMMONCPP2_LIBRARY - The libraries needed to use commoncpp2



if (COMMONCPP2_INCLUDE_DIR AND COMMONCPP2_LIBRARY)
  # Already in cache, be silent
  set(Commoncpp_FIND_QUIETLY TRUE)
endif (COMMONCPP2_INCLUDE_DIR AND COMMONCPP2_LIBRARY)

FIND_PATH(COMMONCPP2_INCLUDE_DIR network.h
  /usr/include/cc++
  /usr/include/
 /usr/local/include/
)

FIND_LIBRARY(COMMONCPP2_LIBRARY NAMES ccgnu2.so
 PATHS
 /usr/lib
 /usr/local/lib
)

IF(COMMONCPP2_FOUND)
  IF(NOT Commoncpp_FIND_QUIETLY)
    MESSAGE(STATUS "Found libcommoncpp2 ${COMMONCPP2_LIBRARY}")
  ENDIF(NOT Commoncpp_FIND_QUIETLY)
ELSE(COMMONCPP2_FOUND)
  IF(Commoncpp_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Commoncpp lib")
  ENDIF(Commoncpp_FIND_REQUIRED)
ENDIF(COMMONCPP2_FOUND)

MARK_AS_ADVANCED( COMMONCPP2_INCLUDE_DIR COMMONCPP2_LIBRARY)
