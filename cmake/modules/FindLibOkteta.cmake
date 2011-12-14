# - Find Okteta libraries, v0 or v1
#
# This module defines
#  LIBOKTETA_FOUND - whether the libokteta libraries were found
#  LIBOKTETA_VERSION - version of the libokteta libraries found
#  LIBOKTETA_LIBRARIES - the libokteta libraries
#  LIBOKTETA_INCLUDE_DIRS - the include paths of the libokteta libraries


if( LIBOKTETA_INCLUDE_DIRS AND LIBOKTETA_LIBRARIES AND LIBOKTETA_VERSION )
    # Already in cache, be silent
    set( Okteta_FIND_QUIETLY TRUE )
endif( LIBOKTETA_INCLUDE_DIRS AND LIBOKTETA_LIBRARIES AND LIBOKTETA_VERSION )

if( LIBOKTETA_FIND_VERSION EQUAL 1 )
    find_library( LIBOKTETACORE1_LIBRARY
        NAMES
        okteta1core
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBOKTETAGUI1_LIBRARY
        NAMES
        okteta1gui
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )


    find_path( LIBOKTETA1_NORMAL_INCLUDE_DIR
        NAMES
        abstractbytearraymodel.h
        PATH_SUFFIXES
        okteta1
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    find_path( LIBOKTETA1_CAMELCASE_INCLUDE_DIR
        NAMES
        Okteta1/AbstractByteArrayModel
        PATH_SUFFIXES
        KDE
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )


    if( LIBOKTETA1_NORMAL_INCLUDE_DIR AND LIBOKTETA1_CAMELCASE_INCLUDE_DIR AND
        LIBOKTETACORE1_LIBRARY AND LIBOKTETAGUI1_LIBRARY )
        set( LIBOKTETA_FOUND TRUE )
    endif( LIBOKTETA1_NORMAL_INCLUDE_DIR AND LIBOKTETA1_CAMELCASE_INCLUDE_DIR AND
        LIBOKTETACORE1_LIBRARY AND LIBOKTETAGUI1_LIBRARY )

    if( LIBOKTETA_FOUND )
        set( LIBOKTETA_VERSION 1 )
        set( LIBOKTETA_INCLUDE_DIRS
            ${LIBOKTETA1_NORMAL_INCLUDE_DIR}
            ${LIBOKTETA1_CAMELCASE_INCLUDE_DIR}
        )
        set( LIBOKTETA_LIBRARIES
            ${LIBOKTETACORE1_LIBRARY}
            ${LIBOKTETAGUI1_LIBRARY}
        )
    endif( LIBOKTETA_FOUND )

endif( LIBOKTETA_FIND_VERSION EQUAL 1 )

if( LIBOKTETA_FIND_VERSION EQUAL 0 )
    find_library( LIBOKTETACORE0_LIBRARY
        NAMES
        oktetacore
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBOKTETAGUI0_LIBRARY
        NAMES
        oktetagui
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_path( LIBOKTETA0_NORMAL_INCLUDE_DIR
        NAMES
        abstractbytearraymodel.h
        PATH_SUFFIXES
        okteta
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    find_path( LIBOKTETA0_CAMELCASE_INCLUDE_DIR
        NAMES
        Okteta/AbstractByteArrayModel
        PATH_SUFFIXES
        KDE
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )


    if( LIBOKTETA0_NORMAL_INCLUDE_DIR AND LIBOKTETA0_CAMELCASE_INCLUDE_DIR AND
        LIBOKTETACORE0_LIBRARY AND LIBOKTETAGUI0_LIBRARY )
        set( LIBOKTETA_FOUND TRUE )
    endif( LIBOKTETA0_NORMAL_INCLUDE_DIR AND LIBOKTETA0_CAMELCASE_INCLUDE_DIR AND
           LIBOKTETACORE0_LIBRARY AND LIBOKTETAGUI0_LIBRARY )

    if( LIBOKTETA_FOUND )
        set( LIBOKTETA_VERSION 0 )
        set( LIBOKTETA_INCLUDE_DIRS
            ${LIBOKTETA0_NORMAL_INCLUDE_DIR}
            ${LIBOKTETA0_CAMELCASE_INCLUDE_DIR}
        )
        set( LIBOKTETA_LIBRARIES
            ${LIBOKTETACORE0_LIBRARY}
            ${LIBOKTETAGUI0_LIBRARY}
        )
    endif( LIBOKTETA_FOUND )
endif( LIBOKTETA_FIND_VERSION EQUAL 0 )


if( LIBOKTETA_FOUND )
    if( NOT Okteta_FIND_QUIETLY )
        message( STATUS "Found Okteta libraries v${LIBOKTETA_VERSION}: ${LIBOKTETA_LIBRARIES}" )
    endif( NOT Okteta_FIND_QUIETLY )
else( LIBOKTETA_FOUND )
    if( LibOkteta_FIND_REQUIRED )
        message( FATAL_ERROR "Could not find Okteta libraries" )
    endif( LibOkteta_FIND_REQUIRED )
endif( LIBOKTETA_FOUND )

mark_as_advanced(
    LIBOKTETA_INCLUDE_DIRS
    LIBOKTETA_LIBRARIES
    LIBOKTETA_VERSION
)
