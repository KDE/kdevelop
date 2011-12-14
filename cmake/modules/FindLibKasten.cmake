# - Find Kasten libraries, v0 or v1
#
# This module defines
#  LIBKASTEN_FOUND - whether the libkasten libraries were found
#  LIBKASTEN_VERSION - version of the libkasten libraries found
#  LIBKASTEN_LIBRARIES - the libkasten libraries
#  LIBKASTEN_INCLUDE_DIRS - the include paths of the libkasten libraries


if( LIBKASTEN_INCLUDE_DIR AND LIBKASTEN_LIBRARIES AND LIBKASTEN_VERSION )
    # Already in cache, be silent
    set( Kasten_FIND_QUIETLY TRUE )
endif( LIBKASTEN_INCLUDE_DIR AND LIBKASTEN_LIBRARIES AND LIBKASTEN_VERSION )

if( LIBKASTEN_FIND_VERSION EQUAL 1 )
    find_library( KASTEN1_CORE_LIB
        NAMES
        kasten1core
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( KASTEN1_GUI_LIB
        NAMES
        kasten1gui
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( KASTEN1_CONTROLLERS_LIB
        NAMES
        kasten1controllers
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_path( KASTEN1_NORMAL_INCLUDE_DIR
        NAMES
        abstractmodel.h
        PATH_SUFFIXES
        kasten1
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    find_path( KASTEN1_CAMELCASE_INCLUDE_DIR
        NAMES
        Kasten1/AbstractModel
        PATH_SUFFIXES
        KDE
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )


    if( KASTEN1_NORMAL_INCLUDE_DIR AND KASTEN1_CAMELCASE_INCLUDE_DIR AND
        KASTEN1_CORE_LIB AND KASTEN1_GUI_LIB AND KASTEN1_CONTROLLERS_LIB )
        set( LIBKASTEN_FOUND TRUE )
    endif( KASTEN1_NORMAL_INCLUDE_DIR AND KASTEN1_CAMELCASE_INCLUDE_DIR AND
        KASTEN1_CORE_LIB AND KASTEN1_GUI_LIB AND KASTEN1_CONTROLLERS_LIB )

    if( LIBKASTEN_FOUND )
        set( LIBKASTEN_VERSION 1 )
        set( LIBKASTEN_INCLUDE_DIRS
            ${KASTEN1_NORMAL_INCLUDE_DIR}
            ${KASTEN1_CAMELCASE_INCLUDE_DIR}
        )
        set( LIBKASTEN_LIBRARIES
            ${KASTEN1_CORE_LIB}
            ${KASTEN1_GUI_LIB}
            ${KASTEN1_CONTROLLERS_LIB}
        )
    endif( LIBKASTEN_FOUND )
endif( LIBKASTEN_FIND_VERSION EQUAL 1 )

if( LIBKASTEN_FIND_VERSION EQUAL 0 )
    find_library( LIBKASTENCORE0_LIBRARY
        NAMES
        kastencore
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBKASTENGUI0_LIBRARY
        NAMES
        kastengui
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBKASTENCONTROLLERS0_LIBRARY
        NAMES
        kastencontrollers
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )


    find_path( LIBKASTEN0_NORMAL_INCLUDE_DIR
        NAMES
        abstractmodel.h
        PATH_SUFFIXES
        kasten
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    find_path( LIBKASTEN0_CAMELCASE_INCLUDE_DIR
        NAMES
        Kasten/AbstractModel
        PATH_SUFFIXES
        KDE
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    if( LIBKASTEN0_NORMAL_INCLUDE_DIR AND LIBKASTEN0_CAMELCASE_INCLUDE_DIR AND
        LIBKASTENCORE0_LIBRARY AND LIBKASTENGUI0_LIBRARY AND LIBKASTENCONTROLLERS0_LIBRARY )
        set( LIBKASTEN_FOUND TRUE )
    endif( LIBKASTEN0_NORMAL_INCLUDE_DIR AND LIBKASTEN0_CAMELCASE_INCLUDE_DIR AND
           LIBKASTENCORE0_LIBRARY AND LIBKASTENGUI0_LIBRARY AND LIBKASTENCONTROLLERS0_LIBRARY )

    if( LIBKASTEN_FOUND )
        set( LIBKASTEN_VERSION 0 )
        set( LIBKASTEN_INCLUDE_DIRS
            ${LIBKASTEN0_NORMAL_INCLUDE_DIR}
            ${LIBKASTEN0_CAMELCASE_INCLUDE_DIR}
        )
        set( LIBKASTEN_LIBRARIES
            ${LIBKASTENCORE0_LIBRARY}
            ${LIBKASTENGUI0_LIBRARY}
            ${LIBKASTENCONTROLLERS0_LIBRARY}
        )
    endif( LIBKASTEN_FOUND )
endif( LIBKASTEN_FIND_VERSION EQUAL 0 )


if( LIBKASTEN_FOUND )
    if( NOT Kasten_FIND_QUIETLY )
        message( STATUS "Found Kasten libraries v${LIBKASTEN_VERSION}: ${LIBKASTEN_LIBRARIES}" )
    endif( NOT Kasten_FIND_QUIETLY )
else( LIBKASTEN_FOUND )
    if( LibKasten_FIND_REQUIRED )
        message( FATAL_ERROR "Could not find Kasten libraries." )
    endif( LibKasten_FIND_REQUIRED )
endif( LIBKASTEN_FOUND )

mark_as_advanced(
    LIBKASTEN_INCLUDE_DIRS
    LIBKASTEN_LIBRARIES
    LIBKASTEN_VERSION
)
