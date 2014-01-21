find_package( LibOktetaKasten )

set_package_properties(LibOktetaKasten PROPERTIES
    PURPOSE "Required for building Okteta KDevelop plugin."
    URL "http://kde.org/"
    TYPE OPTIONAL)

if( LIBOKTETAKASTEN_FOUND )
    set( LIBOKTETA_FIND_VERSION ${LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION} )
    set( LIBKASTEN_FIND_VERSION ${LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION} )
    find_package( LibOkteta )
    find_package( LibKasten )

    set_package_properties(LibOkteta PROPERTIES
        PURPOSE "Required for building Okteta KDevelop plugin."
        URL "http://kde.org/"
        TYPE OPTIONAL)
    set_package_properties(LibKasten PROPERTIES
        PURPOSE "Required for building Okteta KDevelop plugin."
        URL "http://kde.org/"
        TYPE OPTIONAL)
endif( LIBOKTETAKASTEN_FOUND )


if( LIBOKTETAKASTEN_FOUND AND LIBKASTEN_FOUND AND LIBOKTETA_FOUND )
    add_subdirectory(okteta)
endif( LIBOKTETAKASTEN_FOUND AND LIBKASTEN_FOUND AND LIBOKTETA_FOUND )
