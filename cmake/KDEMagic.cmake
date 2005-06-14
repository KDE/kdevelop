
MACRO(KDE_ADD_KPART _target_NAME )
   FOREACH (_current_FILE ${ARGN})

	  GET_FILENAME_COMPONENT(_ext ${_current_FILE} EXT)

      IF(${_ext} STREQUAL ".ui")
         SET(_magic_UIS ${_magic_UIS} ${_current_FILE})
      ENDIF(${_ext} STREQUAL ".ui")

      IF(${_ext} STREQUAL ".c" OR ${_ext} STREQUAL ".C" OR ${_ext} STREQUAL ".cpp" OR ${_ext} STREQUAL ".cxx" )
            SET(_magic_SRCS ${_magic_UIS} ${_current_FILE})
      ENDIF(${_ext} STREQUAL ".c" OR ${_ext} STREQUAL ".C" OR ${_ext} STREQUAL ".cpp" OR ${_ext} STREQUAL ".cxx" )

   ENDFOREACH (_current_FILE)

   IF(_magic_UIS)
      KDE_ADD_UI_FILES(_magic_SRCS ${_magic_UIS} )
   ENDIF(_magic_UIS)

   MESSAGE(STATUS "srcs: ${_magic_SRCS}")

   KDE_AUTOMOC(${_magic_SRCS})
   ADD_LIBRARY(${_target_NAME} MODULE ${_magic_SRCS} )
   KDE_CREATE_LIBTOOL_FILE(${_target_NAME})

ENDMACRO(KDE_ADD_KPART _target_NAME)
