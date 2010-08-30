# - Find Okteta library
#
# This module defines
#  LIBOKTETA_FOUND - whether the libokteta libraries were found
#  LIBOKTETA_LIBRARIES - the libokteta libraries
#  LIBOKTETA_INCLUDE_DIR - the include path of the libokteta libraries


if( LIBOKTETA_INCLUDE_DIR AND LIBOKTETA_LIBRARIES )
    # Already in cache, be silent
    set( Okteta_FIND_QUIETLY TRUE )
endif( LIBOKTETA_INCLUDE_DIR AND LIBOKTETA_LIBRARIES )


find_library( LIBOKTETACORE_LIBRARY
  NAMES
  oktetacore
  HINTS
  ${LIB_INSTALL_DIR}
  ${KDE4_LIB_DIR}
)

find_library( LIBOKTETAGUI_LIBRARY
  NAMES
  oktetagui
  HINTS
  ${LIB_INSTALL_DIR}
  ${KDE4_LIB_DIR}
)

set( LIBOKTETA_LIBRARIES
  ${LIBOKTETACORE_LIBRARY}
  ${LIBOKTETAGUI_LIBRARY}
)


find_path( LIBOKTETA_INCLUDE_DIR
  NAMES
  abstractbytearraymodel.h
  PATH_SUFFIXES
  okteta
  HINTS
  ${INCLUDE_INSTALL_DIR}
  ${KDE4_INCLUDE_DIR}
)

if( LIBOKTETA_INCLUDE_DIR AND LIBOKTETA_LIBRARIES )
   set( LIBOKTETA_FOUND  TRUE )
else( LIBOKTETA_INCLUDE_DIR AND LIBOKTETA_LIBRARIES )
   set( LIBOKTETA_FOUND  FALSE )
endif( LIBOKTETA_INCLUDE_DIR AND LIBOKTETA_LIBRARIES )


if( LIBOKTETA_FOUND )
   if( NOT Okteta_FIND_QUIETLY )
      message( STATUS "Found Okteta libraries: ${LIBOKTETA_LIBRARIES}" )
   endif( NOT Okteta_FIND_QUIETLY )
else( LIBOKTETA_FOUND )
   if( LibOkteta_FIND_REQUIRED )
      message( FATAL_ERROR "Could not find Okteta libraries" )
   endif( LibOkteta_FIND_REQUIRED )
endif( LIBOKTETA_FOUND )

mark_as_advanced( LIBOKTETA_INCLUDE_DIR LIBOKTETA_LIBRARIES )
