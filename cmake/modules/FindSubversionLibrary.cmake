# - Try to find libcommoncpp2
#
#  SUBVERSION_FOUND - system has libcommoncpp2
#  SUBVERSION_INCLUDE_DIR - the commoncpp2 include directory
#  SUBVERSION_ALL_LIBS - The libraries needed to use commoncpp2

IF(SUBVERSION_INCLUDE_DIRS AND SUBVERSION_ALL_LIBS)
    # Already in cache, be silent
    SET(Subversion_FIND_QUIETLY TRUE)
ENDIF(SUBVERSION_INCLUDE_DIRS AND SUBVERSION_ALL_LIBS)

IF (UNIX)

  MACRO(FIND_SUB_LIB targetvar libname)
    IF (SUBVERSION_INSTALL_PATH)
        FIND_LIBRARY(${targetvar} ${libname}
            PATHS
            ${SUBVERSION_INSTALL_PATH}/lib
            NO_DEFAULT_PATH
        )
    ENDIF(SUBVERSION_INSTALL_PATH)
    FIND_LIBRARY(${targetvar} ${libname}
        PATHS
        /usr/lib
        /usr/local/lib
        )
  ENDMACRO(FIND_SUB_LIB)

  FIND_PROGRAM(SVN_ROOT NAMES svn)
  IF (SVN_ROOT)
    STRING(REGEX REPLACE "^(.*)/bin/svn$" "\\1" SVN_ROOT "${SVN_ROOT}")
  ENDIF (SVN_ROOT)

  IF (SUBVERSION_INSTALL_PATH)
    FIND_PATH(SUBVERSION_INCLUDE_DIR subversion-1/svn_client.h
        PATHS
        ${SUBVERSION_INSTALL_PATH}/include
        NO_DEFAULT_PATH
    )
  ENDIF (SUBVERSION_INSTALL_PATH)
  FIND_PATH(SUBVERSION_INCLUDE_DIR subversion-1/svn_client.h
        ${SVN_ROOT}/include
        /usr/include
        /usr/local/include
  )

  # looks like some headers want <header.h> and some want <subversion-1/header.h>
  SET(SUBVERSION_INCLUDE_DIR "${SUBVERSION_INCLUDE_DIR}" "${SUBVERSION_INCLUDE_DIR}/subversion-1")


  FIND_SUB_LIB(SUBVERSION_CLIENTLIB svn_client-1)
  FIND_SUB_LIB(SUBVERSION_REPOSITORYLIB svn_repos-1)
  FIND_SUB_LIB(SUBVERSION_WCLIB svn_wc-1)
  FIND_SUB_LIB(SUBVERSION_FSLIB svn_fs-1)
  FIND_SUB_LIB(SUBVERSION_SUBRLIB svn_subr-1)
  FIND_SUB_LIB(SUBVERSION_RALIB svn_ra-1)

  FIND_PROGRAM(APR_CONFIG NAMES apr-config apr-1-config
    PATHS
    /usr/local/apr/bin
  )

  FIND_PROGRAM(APU_CONFIG NAMES apu-config apu-1-config
    PATHS
    /usr/local/apr/bin
  )

  if(NOT APR_CONFIG)
    MESSAGE(STATUS "no apr-config found, subversion support will be disabled")
    SET(SUBVERSION_FOUND FALSE)
  else(NOT APR_CONFIG)
    if(NOT APU_CONFIG)
      MESSAGE(STATUS "no apu-config found, subversion support will be disabled")
      SET(SUBVERSION_FOUND FALSE)
    else(NOT APU_CONFIG)
      EXEC_PROGRAM(${APR_CONFIG} ARGS "--includedir" OUTPUT_VARIABLE APR_INCLUDE_DIR)
      EXEC_PROGRAM(${APU_CONFIG} ARGS "--includedir" OUTPUT_VARIABLE APU_INCLUDE_DIR)

      EXEC_PROGRAM(${APR_CONFIG} ARGS "--cppflags" OUTPUT_VARIABLE APR_CPP_FLAGS)
      EXEC_PROGRAM(${APU_CONFIG} ARGS "--cppflags" OUTPUT_VARIABLE APU_CPP_FLAGS)

      EXEC_PROGRAM(${APR_CONFIG} ARGS "--ldflags --libs --link-ld" OUTPUT_VARIABLE APR_EXTRA_LIBFLAGS)
      EXEC_PROGRAM(${APU_CONFIG} ARGS "--ldflags --libs --link-ld" OUTPUT_VARIABLE APU_EXTRA_LIBFLAGS)
    endif(NOT APU_CONFIG)
  endif(NOT APR_CONFIG)

  MARK_AS_ADVANCED( APR_CPP_FLAGS
  APU_CPP_FLAGS
  )
ENDIF (UNIX)

#search libaries for Windows
IF (WIN32)

# search for pathes
  FIND_PATH (SUBVERSION_INCLUDE_DIR
    svn_client.h
    "$ENV{ProgramFiles}/Subversion/include"
  )

  FIND_PATH(APR_INCLUDE_DIR apr.h
    "$ENV{ProgramFiles}/Subversion/include/apr"
  )

  FIND_PATH(APU_INCLUDE_DIR apu.h
    "$ENV{ProgramFiles}/Subversion/include/apr-util"
  )


# search for libraries
  FIND_LIBRARY(APR_LIB libapr
    "$ENV{ProgramFiles}/Subversion/lib/apr"
  )

  FIND_LIBRARY(APRICONV_LIB libapriconv
    "$ENV{ProgramFiles}/Subversion/lib/apr-iconv"
  )

  FIND_LIBRARY(APU_LIB libaprutil
    "$ENV{ProgramFiles}/Subversion/lib/apr-util"
  )

  FIND_LIBRARY(APU_XMLLIB xml
    "$ENV{ProgramFiles}/Subversion/lib/apr-util"
  )

  FIND_LIBRARY(NEON_LIB libneon
    "$ENV{ProgramFiles}/Subversion/lib/neon"
  )

  FIND_LIBRARY(NEON_ZLIBSTATLIB zlibstat
    "$ENV{ProgramFiles}/Subversion/lib/neon"
  )

  FIND_LIBRARY(DB43_LIB libdb44
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_CLIENTLIB libsvn_client-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_DELTALIB libsvn_delta-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_DIFFLIB libsvn_diff-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSBASELIB libsvn_fs_base-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSFSLIB libsvn_fs_fs-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSLIB libsvn_fs-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RADAVLIB libsvn_ra_dav-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RALOCALLIB libsvn_ra_local-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RASVNLIB libsvn_ra_svn-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RALIB libsvn_ra-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_REPOSITORYLIB libsvn_repos-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_SUBRLIB libsvn_subr-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_WCLIB libsvn_wc-1
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_INTL3LIB intl3_svn
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  MARK_AS_ADVANCED(
  APRICONV_LIB
  APU_XMLLIB
  NEON_LIB
  NEON_ZLIBSTATLIB
  DB43_LIB
  SUBVERSION_CLIENTLIB
  SUBVERSION_DELTALIB
  SUBVERSION_DIFFLIB
  SUBVERSION_FSBASELIB
  SUBVERSION_FSFSLIB
  SUBVERSION_FSLIB
  SUBVERSION_RADAVLIB
  SUBVERSION_RALOCALLIB
  SUBVERSION_RASVNLIB
  SUBVERSION_INTL3LIB
  SUBVERSION_WCLIB
  SUBVERSION_SUBRLIB
  SUBVERSION_REPOSITORYLIB
  SUBVERSION_RALIB
  )

  SET(APR_EXTRA_LIBFLAGS )
  SET(APU_EXTRA_LIBFLAGS )


# check found libraries
  if (NOT APR_LIB)
    MESSAGE(STATUS "No apr lib found!")
  ELSE (NOT APR_LIB)
    MESSAGE(STATUS "Found apr lib: ${APR_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APR_LIB})
  endif(NOT APR_LIB)

  if (NOT APRICONV_LIB)
    MESSAGE(STATUS "No apriconv lib found!")
  ELSE (NOT APRICONV_LIB)
    MESSAGE(STATUS "Found apriconv lib: ${APRICONV_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APRICONV_LIB})
  endif(NOT APRICONV_LIB)

  if (NOT APU_LIB)
    MESSAGE(STATUS "No aprutil lib found!")
  ELSE (NOT APU_LIB)
    MESSAGE(STATUS "Found aprutil lib: ${APU_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_LIB})
  endif(NOT APU_LIB)

  if (NOT APU_XMLLIB)
    MESSAGE(STATUS "No xml lib found!")
  ELSE (NOT APU_XMLLIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found xml lib: ${APU_XMLLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_XMLLIB})
  endif(NOT APU_XMLLIB)

  if (NOT NEON_LIB)
    MESSAGE(STATUS "No neon lib found!")
  ELSE (NOT NEON_LIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found neon lib: ${NEON_LIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${NEON_LIB})
  endif(NOT NEON_LIB)

  if (NOT NEON_ZLIBSTATLIB)
    MESSAGE(STATUS "No zlibstat lib found!")
  ELSE (NOT APRICONV_LIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found zlibstat lib: ${NEON_ZLIBSTATLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${NEON_ZLIBSTATLIB})
  endif(NOT NEON_ZLIBSTATLIB)

  if (NOT DB43_LIB)
    MESSAGE(STATUS "No db43 lib found!")
    ELSE (NOT DB43_LIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found db43 lib: ${DB43_LIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${DB43_LIB})
  endif(NOT DB43_LIB)

  if (NOT SUBVERSION_DELTALIB)
    MESSAGE(STATUS "No subversion delta lib found!")
  ELSE (NOT SUBVERSION_DELTALIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion delta lib: ${SUBVERSION_DELTALIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_DELTALIB})
  endif(NOT SUBVERSION_DELTALIB)

  if (NOT SUBVERSION_DIFFLIB)
    MESSAGE(STATUS "No subversion diff lib found!")
  ELSE (NOT SUBVERSION_DIFFLIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion diff lib: ${SUBVERSION_DIFFLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_DIFFLIB})
  endif(NOT SUBVERSION_DIFFLIB)

  if (NOT SUBVERSION_FSBASELIB)
    MESSAGE(STATUS "No subversion fs base lib found!")
  ELSE (NOT SUBVERSION_FSBASELIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion fs base lib: ${SUBVERSION_FSBASELIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSBASELIB})
  endif(NOT SUBVERSION_FSBASELIB)

  if (NOT SUBVERSION_FSFSLIB)
    MESSAGE(STATUS "No subversion fs fs lib found!")
  ELSE (NOT SUBVERSION_FSFSLIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion fs fs lib: ${SUBVERSION_FSFSLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSFSLIB})
  endif(NOT SUBVERSION_FSFSLIB)

  if (NOT SUBVERSION_RADAVLIB)
    MESSAGE(STATUS "No subversion ra dav lib found!")
  ELSE (NOT SUBVERSION_RADAVLIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion lib: ${SUBVERSION_RADAVLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RADAVLIB})
  endif(NOT SUBVERSION_RADAVLIB)

  if (NOT SUBVERSION_RALOCALLIB)
    MESSAGE(STATUS "No subversion ra local lib found!")
  ELSE (NOT SUBVERSION_RALOCALLIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion ra local lib: ${SUBVERSION_RALOCALLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RALOCALLIB})
  endif(NOT SUBVERSION_RALOCALLIB)

  if (NOT SUBVERSION_RASVNLIB)
    MESSAGE(STATUS "No subversion ra svn lib found!")
  ELSE (NOT SUBVERSION_RASVNLIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion ra svn lib: ${SUBVERSION_RASVNLIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RASVNLIB})
  endif(NOT SUBVERSION_RASVNLIB)

  if (NOT SUBVERSION_INTL3LIB)
    MESSAGE(STATUS "No subversion intl3 lib found!")
  ELSE (NOT SUBVERSION_INTL3LIB)
    if(NOT Subversion_FIND_QUIETLY)
      MESSAGE(STATUS "Found subversion intl3 lib: ${SUBVERSION_INTL3LIB}")
    endif(NOT Subversion_FIND_QUIETLY)
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_INTL3LIB})
  endif(NOT SUBVERSION_INTL3LIB)


ENDIF (WIN32)


IF(NOT SUBVERSION_INCLUDE_DIR)
  MESSAGE(STATUS "No subversion includes found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE(NOT SUBVERSION_INCLUDE_DIR)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion include: ${SUBVERSION_INCLUDE_DIR}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_FOUND true)
  SET(SUBVERSION_INCLUDE_DIRS ${SUBVERSION_INCLUDE_DIR} ${APR_INCLUDE_DIR} ${APU_INCLUDE_DIR})
ENDIF(NOT SUBVERSION_INCLUDE_DIR)

if (NOT SUBVERSION_CLIENTLIB)
  MESSAGE(STATUS "No subversion client libs found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE (NOT SUBVERSION_CLIENTLIB)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion client lib: ${SUBVERSION_CLIENTLIB}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_CLIENTLIB})
endif(NOT SUBVERSION_CLIENTLIB)

if (NOT SUBVERSION_REPOSITORYLIB)
  MESSAGE(STATUS "No subversion repository lib found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE (NOT SUBVERSION_REPOSITORYLIB)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion repository lib: ${SUBVERSION_REPOSITORYLIB}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_REPOSITORYLIB})
endif(NOT SUBVERSION_REPOSITORYLIB)

if (NOT SUBVERSION_FSLIB)
  MESSAGE(STATUS "No subversion fs lib found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE (NOT SUBVERSION_FSLIB)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion fs lib: ${SUBVERSION_FSLIB}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSLIB})
endif(NOT SUBVERSION_FSLIB)

if (NOT SUBVERSION_SUBRLIB)
  MESSAGE(STATUS "No subversion subr lib found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE (NOT SUBVERSION_SUBRLIB)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion subr lib: ${SUBVERSION_SUBRLIB}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_SUBRLIB})
endif(NOT SUBVERSION_SUBRLIB)

if (NOT SUBVERSION_WCLIB)
  MESSAGE(STATUS "No subversion wc lib found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE (NOT SUBVERSION_WCLIB)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion wc lib: ${SUBVERSION_WCLIB}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_WCLIB})
endif(NOT SUBVERSION_WCLIB)

if (NOT SUBVERSION_RALIB)
  MESSAGE(STATUS "No subversion ra lib found, subversion support will be disabled")
  SET(SUBVERSION_FOUND false)
ELSE (NOT SUBVERSION_RALIB)
  if(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Found subversion ra lib: ${SUBVERSION_RALIB}")
  endif(NOT Subversion_FIND_QUIETLY)
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RALIB})
endif(NOT SUBVERSION_RALIB)

SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} CACHE STRING "List of all subversion and related libraries")
SET(SUBVERSION_INCLUDE_DIRS ${SUBVERSION_INCLUDE_DIRS} CACHE STRING "List of all subversion and related libraries include directories")
SET(SUBVERSION_INCLUDE_DIR ${SUBVERSION_INCLUDE_DIR} CACHE PATH "Path of subversion include directory")

MARK_AS_ADVANCED(
  SUBVERSION_RALIB
  SUBVERSION_WCLIB
  SUBVERSION_SUBRLIB
  SUBVERSION_FSLIB
  SUBVERSION_CLIENTLIB
  SUBVERSION_REPOSITORYLIB
  SUBVERSION_INCLUDE_DIR
  SUBVERSION_INCLUDE_DIRS
  SUBVERSION_ALL_LIBS
  APR_INCLUDE_DIR
  APR_EXTRA_LIBFLAGS
  APU_INCLUDE_DIR
  APU_EXTRA_LIBFLAGS
)

#kate: space-indent on; indent-width 2; tab-width: 2; replace-tabs on; auto-insert-doxygen on