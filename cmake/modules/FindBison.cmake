# - Try to find Bison
# Once done this will define
#
#  BISON_FOUND - system has Bison
#  BISON_EXECUTABLE - path of the bison executable
#  BISON_VERSION - the version string, like "2.5.31"
#


FIND_PROGRAM(BISON_EXECUTABLE NAMES bison)

INCLUDE(MacroEnsureVersion)

IF(BISON_EXECUTABLE)
    SET(BISON_FOUND TRUE)

    EXECUTE_PROCESS(COMMAND ${BISON_EXECUTABLE} --version
        OUTPUT_VARIABLE _BISON_VERSION
    )
    string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" BISON_VERSION "${_bison_VERSION}")
ENDIF(BISON_EXECUTABLE)

IF(BISON_FOUND)
  IF(NOT Bison_FIND_QUIETLY)
    MESSAGE(STATUS "Found Bison: ${BISON_EXECUTABLE}")
  ENDIF(NOT Bison_FIND_QUIETLY)
ELSE(BISON_FOUND)
  IF(Bison_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Bison")
  ENDIF(Bison_FIND_REQUIRED)
ENDIF(BISON_FOUND)

